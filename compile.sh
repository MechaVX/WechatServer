cd build
cmake ..

if [ $? -ne 0 ]; then
    exit 1
fi

make

if [ $? -ne 0 ]; then
    exit 1
fi

if [ -n $1 ]; then
    exit 0
fi
EXE=$1
${EXE}