if [ $# -ge 1 ]; then
    cmd=$1
else
    cmd="start"
fi

if [ $cmd == "start" ]; then
    docker run -d --rm -it -v `pwd`:/root/work/PP-CNN --name ppcnn ppcnn
elif [ $cmd == "stop" ]; then
    docker stop ppcnn
fi
