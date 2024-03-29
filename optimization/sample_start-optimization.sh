#!/bin/bash

# Choose which optimization task to run
#task="walk"
task="walk"


# This script runs the simspark soccer simulator and an agent

# Set the agent and monitor port randomly, to allow for multiple agents per machine
# Note: $RANDOM returns a value from 0 to 32767, ports <= 1024 are reserved for root
# TODO: Instead of picking random ports purposely bind to available ports
# export SPARK_SERVERPORT=$[$RANDOM + 1025] #3200
# export SPARK_AGENTPORT=$[$RANDOM + 1025] #3100
export SPARK_SERVERPORT=$[3200]
export SPARK_AGENTPORT=$[3100]

echo -n "It is: "
date
echo -n "and I am on: "
hostname
echo "Agent port: $SPARK_AGENTPORT, Monitor port: $SPARK_SERVERPORT"

rcssserver3d --agent-port $SPARK_AGENTPORT --server-port $SPARK_SERVERPORT --script-path rcssserver3d.rb > /dev/null 2> /dev/null &
PID=$(echo $!)

#To view task while it runs uncomment the following line
#<roboviz_start_script> --serverPort=$SPARK_SERVERPORT &

sleep 3
NUM_PLAYERS=7
DIR_SCRIPT="$( cd "$( dirname "$0" )" && pwd )"
TYPE=$1
DIR_PARAMS="$( cd "$( dirname "$2" )" && pwd )"
DIR_OUTPUT="$( cd "$( dirname "$3" )" && pwd )"
PARAMS_FILE=$DIR_PARAMS/$(basename $2)
OUTPUT_FILE=$DIR_OUTPUT/$(basename $3)

# TODO: temporary fix!! get rid of it - because relative path dependency of skills
cd $DIR_SCRIPT/..
export LD_LIBRARY_PATH=./libs:$LD_LIBRARY_PATH



if [ $task == "kick" ]
then
    # FixedKick optimization task
    $DIR_SCRIPT/../agentspark --unum 2 --type $TYPE --paramsfile $DIR_SCRIPT/../paramfiles/defaultParams.txt --paramsfile $DIR_SCRIPT/../paramfiles/defaultParams_t$TYPE.txt --paramsfile $PARAMS_FILE --experimentout $OUTPUT_FILE --optimize fixedKickAgent --port $SPARK_AGENTPORT --mport $SPARK_SERVERPORT &
fi

if [ $task == "walk" ]
then
    # WalkForward optimization task
    $DIR_SCRIPT/../agentspark --unum 2 --type $TYPE --paramsfile $DIR_SCRIPT/../paramfiles/defaultParams.txt --paramsfile $DIR_SCRIPT/../paramfiles/defaultParams_t$TYPE.txt --paramsfile $PARAMS_FILE --experimentout $OUTPUT_FILE --optimize walkForwardAgent --port $SPARK_AGENTPORT --mport $SPARK_SERVERPORT &
fi

AGENTPID=$!
sleep 2

maxWaitTimeSecs=60
total_wait_time=0

originLine=$(cat $OUTPUT_FILE |wc -l)
line=$originLine

# echo "lien line line line $line"
# echo "cnt  cnt  cnt  cnt  $cnt "

while [ $line -ne $(($originLine+1)) ] && [ $total_wait_time -lt $maxWaitTimeSecs ]
do
  line=$(cat $OUTPUT_FILE |wc -l)
  sleep 1
  total_wait_time=`expr $total_wait_time + 1`
done

echo "Killing Simulator"
kill -9 $PID >/dev/null 2>/dev/null
echo "Killing Agent"
kill -9 $AGENTPID >/dev/null 2>/dev/null

sleep 2
echo "Finished"
