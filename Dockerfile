FROM centos:7

RUN yum -y install gcc-c++ boost boost-devel wget make openssl-devel git bzip2
RUN cd tmp && \
    wget https://github.com/Kitware/CMake/releases/download/v3.18.4/cmake-3.18.4.tar.gz && \
    tar xvfz cmake-3.18.4.tar.gz && cd cmake-3.18.4 && \
    ./bootstrap && make -j8 && make install
RUN cd tmp && \
    wget https://support.hdfgroup.org/ftp/lib-external/szip/2.1.1/src/szip-2.1.1.tar.gz && \
    tar xvfz szip-2.1.1.tar.gz && cd szip-2.1.1 && \
    ./configure --prefix=/usr/local && make -j8 && make install
ENV LD_LIBRARY_PATH $LD_LIBRARY_PATH:/usr/local/lib
RUN cd tmp && \
    wget https://hdf-wordpress-1.s3.amazonaws.com/wp-content/uploads/manual/HDF5/HDF5_1_12_0/source/hdf5-1.12.0.tar.gz && \
    tar xvfz hdf5-1.12.0.tar.gz && cd hdf5-1.12.0 && \
    ./configure --prefix=/usr/local/hdf5 --enable-cxx --with-szlib=/usr/local/lib --enable-threadsafe --with-pthread=/usr/include/ --enable-hl --enable-shared --enable-unsupported && \
    make -j8 && make install
ENV PATH $PATH:/usr/local/hdf5/bin
ENV LIBRARY_PATH $LIBRARY_PATH:/usr/local/hdf5/lib
ENV LD_LIBRARY_PATH $LD_LIBRARY_PATH:/usr/local/hdf5/lib
RUN cd tmp && \
    wget http://ftp.tsukuba.wide.ad.jp/software/gcc/releases/gcc-7.4.0/gcc-7.4.0.tar.gz && \
    tar xvfz gcc-7.4.0.tar.gz && cd gcc-7.4.0 && \
    ./contrib/download_prerequisites && \
    mkdir build && cd build && \
    ../configure --enable-languages=c,c++ --prefix=/usr/local --disable-bootstrap --disable-multilib && \
    make -j8 && make install
ENV PATH /usr/locanl/bin:$PATH
RUN yum -y remove gcc-c++
RUN cd tmp && \
    wget https://github.com/microsoft/SEAL/archive/v3.4.4.tar.gz && \
    tar xvfz v3.4.4.tar.gz && cd SEAL-3.4.4/native/src && cd SEAL-3.4.4/native/src && \
    cmake . && make -j8 && make install
    
WORKDIR /root
COPY . /root

RUN cd pp_cnn/src
RUN mkdir build && cd build
RUN cmake .. && make -j8
