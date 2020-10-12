if [ $1 == "start" ]; then
    docker run -d --rm -it --name ppcnn ppcnn
elif [ $1 == "stop" ]; then
    docker stop ppcnn
fi
