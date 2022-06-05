source env/bin/activate
source ../env_sensorwriter.sh

python receiver.py > receiver.log 2> receiver_error.log