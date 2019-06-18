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
    outputFile( outputFile_ ), TOTAL_WALK_TIME(20.) {


    INIT_WAIT = 3;
    run = 0;
    totalWalkTime = 0;
    worldModel->setUNum(uNum);
    // Use ground truth localization for behavior
    worldModel->setUseGroundTruthDataForLocalization(true);
    // TODO: MYSQL INIT
    init();
}

void OptimizationBehaviorWalkForward::init() {
    startTime = worldModel->getTime();
    initialized = false;
    initBeamed = false;
    beamChecked = false;
    fallen = false;
    string msg = "(playMode BeforeKickOff)";

//    cout<<"No."<<worldModel->getUNum() <<" : wait time "<<INIT_WAIT<<endl;
    if (worldModel->getUNum() == 7)
    setMonMessage(msg);
}

void OptimizationBehaviorWalkForward::beam( double& beamX, double& beamY, double& beamAngle ) {
    beamX = -HALF_FIELD_X+4;
    beamY = 1.5*worldModel->getUNum()-8;
    target = VecPosition(0, beamY, 0);
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
    if(distance > 0.1 || angleOffset > 5) {
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
    if (currentTime-startTime < INIT_WAIT || startTime < 0) {
//        if (currentTime-startTime > INIT_WAIT + TOTAL_WALK_TIME)
//        cout<<worldModel->getUNum()<<" : time over, stop at "<<worldModel->getTime()<<endl;
        return SKILL_STAND;
    }
//    if (currentTime-startTime > INIT_WAIT + TOTAL_WALK_TIME)
//        cerr<<"walk time is over!\n";
    return goToTarget(target);
}

void OptimizationBehaviorWalkForward::
updateFitness() {
    static bool written = false;
    const int PLAY_MODE = worldModel->getPlayMode();
    if (run == 2) {
        if (!written) {
            double fitness = totalWalkTime/(double)run;
            fstream file;
            file.open(outputFile.c_str(), ios::app );
            file << fitness << '\n';
            file.close();
            written = true;
//            cout<<"No."<<worldModel->getUNum()<<" write data at "<<worldModel->getTime()<<endl;
        }
        return;
    }

    if (startTime < 0 /*|| (PLAY_MODE != PM_PLAY_ON && worldModel->getUNum() != 7*/) {
        init();
        return;
    }

//    if (worldModel->getUNum() == 7) cout<<"i am here\n";
    static bool started = false;
    if (!started && PLAY_MODE == PM_PLAY_ON)  //
    {
        startTime = worldModel->getTime();
        started = ~started;
//        cout <<"No."<<worldModel->getUNum()<<" started at "<<startTime<<endl;
    }

    double currentTime = worldModel->getTime();
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
                totalWalkTime += 100;
                run++;
                LOG_STR("failed to beamcheck");
            }
            failedLastBeamCheck = true;
            init();
            return;
        } else {
            failedLastBeamCheck = false;
            // Set playmode to PlayOn to start run and move ball out of the way
//            string msg = "(ball (pos 0 -9 0) (vel 0 0 0))";
            string msg = "(playMode PlayOn) (ball (pos 0 -9 0) (vel 0 0 0))";
            if (worldModel->getUNum() == 7)
            setMonMessage(msg);
        }
    }
    if (!fallen && worldModel->isFallen())
        fallen = true;
//    VecPosition accel = bodyModel->getAccelRates();
//    static fstream pose("pose", ios::app);
//    pose << accel.getX()<<","<<accel.getY()<<","<<accel.getZ()<<endl;
    double lastTime = TOTAL_WALK_TIME+INIT_WAIT;
    lastTime += (worldModel->getUNum() == 7 ? 1 : 0);
    if (currentTime-startTime >= lastTime || me.getDistanceTo(target) < .3) {
//        pose << "----------------\n";
//        pose.close();
        VecPosition me = worldModel->getMyPositionGroundTruth();
        double beamX, beamY, beamAngle;
        beam(beamX, beamY, beamAngle);
        VecPosition start = VecPosition(beamX, beamY, 0);

        static double walkTime;
        walkTime = currentTime-startTime-INIT_WAIT;
        cout <<"No."<<worldModel->getUNum()<< " Run " << run << " time walked: " << walkTime << endl;
        if (fallen)
            totalWalkTime += 50;
        else
            totalWalkTime += walkTime;
        run++;
        started = false;
        init();
    }

}
