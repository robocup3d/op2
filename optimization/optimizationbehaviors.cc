#include "optimizationbehaviors.h"
#include <fstream>


/*
 *
 *
 * Fixed Kick optimization agent
 *
 *
 */
OptimizationBehaviorFixedKick::OptimizationBehaviorFixedKick(const std::string teamName,
        int uNum, const map<string, string>& namedParams_, const string& rsg_,
        const string& outputFile_) :
    NaoBehavior(teamName, uNum, namedParams_, rsg_), outputFile(outputFile_),
    kick(0), INIT_WAIT_TIME(3.0)
{
    initKick();
}

void OptimizationBehaviorFixedKick::beam(double& beamX, double& beamY,
        double& beamAngle) {
    beamX = atof(namedParams.find("kick_xoffset")->second.c_str());
    beamY = atof(namedParams.find("kick_yoffset")->second.c_str());
    beamAngle = atof(namedParams.find("kick_angle")->second.c_str());
}

SkillType OptimizationBehaviorFixedKick::selectSkill() {
    double time = worldModel->getTime();
    if (timeStart < 0) {
        initKick();
        return SKILL_STAND;
    }

    // Wait a bit before attempting kick
    if (time-timeStart <= INIT_WAIT_TIME) {
        return SKILL_STAND;
    }

    if (!hasKicked) {
        hasKicked = true;
        return SKILL_KICK_LEFT_LEG; // The kick skill that we're optimizing
    }

    return SKILL_STAND;
}

void OptimizationBehaviorFixedKick::updateFitness() {
    static double totalFitness = 0.0;
    if (kick == 10) {
        writeFitnessToOutputFile(totalFitness/(double(kick)));
        return;
    }

    double time = worldModel->getTime();
    VecPosition meTruth = worldModel->getMyPositionGroundTruth();
    meTruth.setZ(0);

    if (time-timeStart <= INIT_WAIT_TIME) {
        return;
    }

    static bool failedLastBeamCheck = false;
    if(!beamChecked) {
        cout << "Checking whether beam was successful\n";
        beamChecked = true;
        LOG_STR("Checking whether beam was successful");

        meTruth.setZ(0);
        double beamX, beamY, beamAngle;
        beam(beamX, beamY, beamAngle);
        VecPosition meDesired = VecPosition(beamX, beamY, 0);
        double distance = meTruth.getDistanceTo(meDesired);
        double angle = worldModel->getMyAngDegGroundTruth();
        VecPosition ballPos = worldModel->getBallGroundTruth();
        double ballDistance = ballPos.getMagnitude();

        // Check that we're close to our expected position and angle
        // and also that the ball is close to it's exepected position
        if(distance > .1 || ballDistance > .1 || abs(angle - beamAngle) > 3) {
            cout << distance << "\t" << ballDistance << "\n";
            LOG_STR("Problem with the beam!");
            LOG(distance);
            LOG(meTruth);
            LOG(meDesired);
            if (failedLastBeamCheck) {
                kick++;
                totalFitness -= 100;
                failedLastBeamCheck = false;
            } else {
                failedLastBeamCheck = true;
            }
            initKick();
            return;
        }
        failedLastBeamCheck = false;
        string msg = "(playMode KickOff_Left)";
        setMonMessage(msg);
    }

    if (!hasKicked && worldModel->getPlayMode() == PM_PLAY_ON) {
        ranIntoBall = true;
    }

    if (!hasKicked) {
        return;
    }

    VecPosition ballTruth = worldModel->getBallGroundTruth();
    if (ballTruth.getX() < -.25) {
        backwards = true;
    }

    if (worldModel->isFallen()) {
        fallen = true;
    }

    if (worldModel->isFallen()) {
        totalFitness += -1;
        kick++;
        initKick();
        return;
    }

    if (time - (timeStart + INIT_WAIT_TIME) > 15 && !isBallMoving(this->worldModel)) {
//        double angleOffset = abs(VecPosition(0, 0, 0).getAngleBetweenPoints(VecPosition(20, 0, 0), ballTruth));
        double distance = ballTruth.getX();
        double fitness = distance;

        if (backwards || distance <= 0.1 || ranIntoBall) {
            fitness = -100;
            if (backwards) {
                cout << "Detected backward kick" << endl;
            } else if (ranIntoBall) {
                cout << "Detected ranIntoBall" << endl;
            } else {
                cout << "Detected insufficient distance" << endl;
            }
        }
        cout << "Traveled distance = " << distance << endl;
        cout << "Fitness = " << fitness << endl;
        cout << "Final position = " << ballTruth.getX() << ", " << ballTruth.getY() << endl;

        totalFitness += fitness;
        kick++;
        initKick();
        return;
    }
}

void OptimizationBehaviorFixedKick::initKick() {
    hasKicked = false;
    beamChecked = false;
    backwards = false;
    ranIntoBall = false;
    timeStart = worldModel->getTime();
    initialized = false;
    initBeamed = false;
    fallen = false;
    resetSkills();

    // Beam agent and ball
    double beamX, beamY, beamAngle;
    beam(beamX, beamY, beamAngle);
    VecPosition beamPos = VecPosition(beamX, beamY, 0);
    string msg = "(playMode BeforeKickOff)";
    setMonMessage(msg);
}

void OptimizationBehaviorFixedKick::writeFitnessToOutputFile(double fitness) {
    static bool written = false;
    if (!written) {
        LOG(fitness);
        LOG(kick);
        fstream file;
        file.open(outputFile.c_str(), ios::out);
        file << fitness << endl;
        file.close();
        written = true;
        //string msg = "(killsim)";
        //setMonMessage(msg);
    }
}


/* Checks if the ball is currently moving */
bool isBallMoving(const WorldModel *worldModel) {
    static VecPosition lastBall = worldModel->getBallGroundTruth();
    static double lastTime = worldModel->getTime();

    double thisTime = worldModel->getTime();
    VecPosition thisBall = worldModel->getBallGroundTruth();

    thisBall.setZ(0);
    lastBall.setZ(0);

    if(thisBall.getDistanceTo(lastBall) > 0.01)
    {
        // the ball moved!
        lastBall = thisBall;
        lastTime = thisTime;
        return true;
    }

    if(thisTime - lastTime < 0.5)
    {
        // not sure yet if the ball has settled
        return true;
    }
    else
    {
        return false;
    }
}


void writeToOutputFile(const string &filename, const string &output) {
//  static bool written = false;
//  assert(!written);
    //  LOG(output);
    fstream file;
    file.open(filename.c_str(), ios::out);
    file << output;
    file.close();
//  written = true;
}


/*
 *
 *
 * WALK FORWARD OPTIMIZATION AGENT
 *
 *
 *
 */
OptimizationBehaviorWalkForward::
OptimizationBehaviorWalkForward( const std::string teamName,
                                 int uNum,
                                 const map<string, string>& namedParams_,
                                 const string& rsg_,
                                 const string& outputFile_)
    : NaoBehavior( teamName,
                   uNum,
                   namedParams_,
                   rsg_ ),
    outputFile( outputFile_ ),
//    TOTAL_WALK_TIME(20.),
    MAX_WAIT(120)
    {
    INIT_WAIT = 3;
    run = 0;
    totalWalkTime = 0;
    worldModel->setUNum(uNum);
    roboviz = worldModel->getRVSender();
    roboviz->clear();
    // Use ground truth localization for behavior
    worldModel->setUseGroundTruthDataForLocalization(true);
    init();
    target = goals.back();
}

void OptimizationBehaviorWalkForward::init() {
    static int _goals[16][2] = {
            {-8,4},{-5,5},{-2,4},{0,0},{2,-4},
            {5,-5},{8,-4},{10,0},{8,4},{5,5},
            {2,4},{0,0},{-2,-4},{-5,-5},{-8,-4},{-10,0}
    };
    target_num = 0;
    startTime = worldModel->getTime();
    initialized = false;
    initBeamed = false;
    beamChecked = false;
    fallen = false;
    fallen_count = 0;
    display_count = 0;
    string msg = "(playMode BeforeKickOff)";
    goals.clear();
    for (int i=15;i>=0;i--)
    {
        goals.push_back(VecPosition(_goals[i][0],_goals[i][1],0));
    }
    setMonMessage(msg);
}

void OptimizationBehaviorWalkForward::beam( double& beamX, double& beamY, double& beamAngle ) {
    beamX = -HALF_FIELD_X+5;
    beamY = 0;
    beamAngle = 0;
}

bool OptimizationBehaviorWalkForward::checkBeam() {
//    LOG_STR("Checking whether beam was successful");
    VecPosition meTruth = worldModel->getMyPositionGroundTruth();
    meTruth.setZ(0);
    double beamX, beamY, beamAngle;
    beam(beamX, beamY, beamAngle);
    VecPosition meDesired = VecPosition(beamX, beamY, 0);
    double distance = meTruth.getDistanceTo(meDesired);
    double angleOffset = abs(worldModel->getMyAngDegGroundTruth()-beamAngle);
    // enlarge error tolerance
    if(distance > 1 || angleOffset > 25) {
        LOG_STR("Problem with the beam!");
        LOG(distance);
        LOG(meTruth);

        return false;
    }
    beamChecked = true;
    return true;
}

SkillType OptimizationBehaviorWalkForward::selectSkill() {
    double currentTime = worldModel->getTime();
    if (currentTime-startTime < INIT_WAIT || startTime < 0 || goals.empty()) {
//        if (currentTime-startTime > INIT_WAIT + TOTAL_WALK_TIME)
//        cout<<worldModel->getUNum()<<" : time over, stop at "<<worldModel->getTime()<<endl;
        return SKILL_STAND;
    }
//    if (currentTime-startTime > INIT_WAIT + TOTAL_WALK_TIME)
//        cerr<<"walk time is over!\n";
    if (me.getDistanceTo(target) < .5)
    {
        if (!goals.empty())
        {
            goals.pop_back();
            target_num++;
            // GET REWARD
            totalWalkTime -= 5;
        }

        if (goals.empty())
        {
            run++;
            double walkTime;
            walkTime = currentTime-startTime+INIT_WAIT;
            cout <<"\t>>>  No."<<worldModel->getUNum()<< " Run " << run << " time cost: " << walkTime << endl;
            totalWalkTime += walkTime;
            init();
        }
    }
    target = goals.back();
    return goToTarget(target);
}

void OptimizationBehaviorWalkForward::
updateFitness() {
    static bool written = false;
    const int PLAY_MODE = worldModel->getPlayMode();
    static bool started = false;
    double currentTime = worldModel->getTime();

    if (run == 5) {
        if (!written) {
            double fitness = totalWalkTime/run;
            fstream file;
            file.open(outputFile.c_str(), ios::app );
            file << fitness << '\n';
            file.close();
            written = true;
//            cout<<"No."<<worldModel->getUNum()<<" write data at "<<worldModel->getTime()<<endl;
        }

        //// TODO: ADD CODES HERE TO CONTINUE RUNNING

        return;
    }

//    worldModel->getRVSender()->drawText("display test",0,0,RVSender::RED);
//    roboviz->drawAgentText("display test",2,SIDE_LEFT,RVSender::RED);

#define MESSAGE_PCS 4
    static char rvMSG[MESSAGE_PCS][50];

    if (display_count % 50 == 0)
    {
        roboviz->clearStaticDrawings();
        roboviz->drawCircle(target.getX(),target.getY(),.5,RVSender::BLUE);
        sprintf(rvMSG[0], "OPTIMIZATION STATUS");
        sprintf(rvMSG[1], "Agent Type: %d", agentType);
        sprintf(rvMSG[2], "Round: %d", run);
        sprintf(rvMSG[3], "Real Time cost: %.2f", currentTime-startTime+INIT_WAIT);
        for (int i=0;i<MESSAGE_PCS;i++)
            roboviz->drawText(rvMSG[i], 0, 20-3*i);
    }


    if (startTime < 0 /*|| (PLAY_MODE != PM_PLAY_ON && worldModel->getUNum() != 7*/) {
        init();
        return;
    }


    if (!started && PLAY_MODE == PM_PLAY_ON)  //
    {
        startTime = worldModel->getTime();
        started = ~started;
    }


    if (currentTime-startTime < INIT_WAIT) {
        return;
    }


    if (!beamChecked) {
        static bool failedLastBeamCheck = false;
        if (!checkBeam()) {
            // Beam failed so reinitialize everything
            if (failedLastBeamCheck) {
                // Probably something bad happened if we failed the beam twice in
                // a row (perhaps the agent can't stand) so give a bad score and
                // move on
                totalWalkTime += 1000;
                run++;
                LOG_STR("failed to beamcheck");
            }
            failedLastBeamCheck = true;
            init();
            return;
        } else {
            failedLastBeamCheck = false;
            // Set playmode to PlayOn to start run and move ball out of the way
            string msg = "(playMode PlayOn) (ball (pos 0 -9 0) (vel 0 0 0))";
            setMonMessage(msg);
        }
    }
    if ( currentTime-startTime > INIT_WAIT + MAX_WAIT)
    {
        cout <<"\t>>>  No."<<worldModel->getUNum()<< " Time out, " << run << " time cost: 1000" << endl;
        run++;
        totalWalkTime += 1000;
        started = false;
        init();
        return;
    }
//    static int fallen_count;
    if(!fallen && worldModel->isFallen())
    {
        double time_cost = 1000*(++fallen_count)-(10+me.getX())*90;
        if (fallen || fallen_count > 0)
        {
            cout <<"\t>>>  No."<<worldModel->getUNum()<< " Fallen, " << run << " time cost "<<time_cost << endl;
            run++;
            started = false;
            init();
            return;
        }
        fallen = true;

        cout <<"\t>>>  No."<<worldModel->getUNum()<< " Fallen, " << run << " time cost add "<<time_cost << endl;
        totalWalkTime += time_cost;
        return;
    }
    else
        fallen = false;

    VecPosition me = worldModel->getMyPositionGroundTruth();
    double distance = me.getDistanceTo(target);
    display_count++;
    if (display_count % 10 == 0)
    cout<<"\rdistance to target "<<target_num<< " is "<<distance;
    if (currentTime-startTime >= MAX_WAIT+INIT_WAIT || goals.empty()) {

        static double walkTime;
        walkTime = currentTime-startTime+INIT_WAIT;
        cout <<"\t>>>  No."<<worldModel->getUNum()<< " Run " << run << " time cost: " << walkTime << endl;
        if (fallen)
            totalWalkTime += 1000;
        else
            totalWalkTime += walkTime;
        run++;
        started = false;
        init();
    }

}
