FROM centos:7

RUN yum install -y gcc-c++ wget make openssl-devel git bzip2 which mlocate
RUN yum install -y centos-release-scl
RUN yum install -y devtoolset-8-gcc devtoolset-8-gcc-c++

SHELL ["/usr/bin/scl", "enable", "devtoolset-8"]

RUN cd tmp \
    && wget https://dl.bintray.com/boostorg/release/1.74.0/source/boost_1_74_0.tar.gz \
    && tar xvfz boost_1_74_0.tar.gz \
    && cd boost_1_74_0 \
    && ./bootstrap.sh --without-icu --with-libraries=context,filesystem,graph,iostreams,program_options \
    && ./b2 -j4 link=static,shared runtime-link=shared threading=multi variant=release --layout=tagged --build-dir=../b2gcc --stagedir=stage/gcc stage \
    && ./b2 -j4 --prefix=/usr install \
    && ldconfig
#    && ./bootstrap.sh --without-icu --with-libraries=context,filesystem,graph,iostreams,program_options,serialization,system,test
#    && ./bootstrap.sh \
#    && ./b2 --without-python --prefix=/usr -j 4 link=shared runtime-link=shared install

RUN cd tmp \
    && wget https://github.com/Kitware/CMake/releases/download/v3.18.4/cmake-3.18.4.tar.gz \
    && tar xvfz cmake-3.18.4.tar.gz && cd cmake-3.18.4 \
    && ./bootstrap \
    && make -j4 \
    && make install
RUN cd tmp \
    && wget https://support.hdfgroup.org/ftp/lib-external/szip/2.1.1/src/szip-2.1.1.tar.gz \
    && tar xvfz szip-2.1.1.tar.gz && cd szip-2.1.1 \
    && ./configure --prefix=/usr/local \
    && make -j4 \
    && make install
ENV LD_LIBRARY_PATH $LD_LIBRARY_PATH:/usr/local/lib
RUN cd tmp \
    && wget https://hdf-wordpress-1.s3.amazonaws.com/wp-content/uploads/manual/HDF5/HDF5_1_12_0/source/hdf5-1.12.0.tar.gz \
    && tar xvfz hdf5-1.12.0.tar.gz && cd hdf5-1.12.0 \
    && ./configure --prefix=/usr/local/hdf5 --enable-cxx --with-szlib=/usr/local/lib --enable-threadsafe --with-pthread=/usr/include/ --enable-hl --enable-shared --enable-unsupported \
    && make -j4 \
    && make install

RUN cd tmp \
    && wget https://github.com/microsoft/SEAL/archive/v3.4.4.tar.gz \
    && tar xvfz v3.4.4.tar.gz && cd SEAL-3.4.4/native/src \
    && cmake . \
    && make -j4 \
    && make install

ENV PATH $PATH:/usr/local/hdf5/bin
ENV LIBRARY_PATH $LIBRARY_PATH:/usr/local/hdf5/lib
ENV LD_LIBRARY_PATH /usr/lib:$LD_LIBRARY_PATH:/usr/local/hdf5/lib

WORKDIR /root
COPY . /root

RUN cd pp_cnn/src \
    && mkdir build \
    && cd build \
    && cmake .. \
    && make

RUN mkdir -p pp_cnn/datasets/mnist && cd pp_cnn/datasets/mnist \
    && wget http://yann.lecun.com/exdb/mnist/train-images-idx3-ubyte.gz \
    && wget http://yann.lecun.com/exdb/mnist/train-labels-idx1-ubyte.gz \
    && wget http://yann.lecun.com/exdb/mnist/t10k-images-idx3-ubyte.gz \
    && wget http://yann.lecun.com/exdb/mnist/t10k-labels-idx1-ubyte.gz \
    && gunzip *.gz
RUN mkdir -p pp_cnn/datasets/cifar-10 && cd pp_cnn/datasets/cifar-10 \
    && wget https://www.cs.toronto.edu/~kriz/cifar-10-binary.tar.gz \
    && tar zxvf cifar-10-binary.tar.gz \
    && mv cifar-10-batches-bin/* ./ && rmdir cifar-10-batches-bin
RUN mkdir pp_cnn/logs && touch pp_cnn/logs/main_log.txt