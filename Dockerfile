FROM centos:7

RUN yum -y install gcc-c++ boost boost-devel wget make openssl-devel git
RUN cd tmp && \
    wget https://github.com/Kitware/CMake/releases/download/v3.18.4/cmake-3.18.4.tar.gz && \
    tar xvfz cmake-3.18.4.tar.gz && cd cmake-3.18.4 && \
    ./bootstrap && make -j8 && make install
RUN cd tmp && \
    wget https://support.hdfgroup.org/ftp/lib-external/szip/2.1.1/src/szip-2.1.1.tar.gz && \
    tar xvfz szip-2.1.1.tar.gz && cd szip-2.1.1 && \
    ./configure --prefix=/usr/local && make -j8 && make install &&
ENV LD_LIBRARY_PATH $LD_LIBRARY_PATH:/usr/local/lib
RUN cd tmp && \
    wget https://hdf-wordpress-1.s3.amazonaws.com/wp-content/uploads/manual/HDF5/HDF5_1_12_0/source/hdf5-1.12.0.tar.gz && \
    tar xvfz hdf5-1.12.0.tar.gz && cd hdf5-1.12.0 && \
    ./configure --prefix=/usr/local/hdf5 --enable-cxx --with-szlib=/usr/local/lib --enable-threadsafe --with-pthread=/usr/include/ --enable-hl --enable-shared --enable-unsupported && \
    make -j8 && make install
    
WORKDIR /root
COPY . /root

#RUN apt-get update
#RUN apt-get -y upgrade
#RUN apt-get install -y tzdata
#RUN apt-get install -y git vim mosquitto python3 python3-pip
#
## language setting
#RUN apt-get install -y language-pack-ja-base language-pack-ja
#RUN locale-gen en_US.UTF-8
#ENV LANG en_US.UTF-8
#ENV LANGUAGE en_US:en
#ENV LC_ALL en_US.UTF-8
#
## python setting
#RUN ln -s /usr/bin/python3 /usr/bin/python && \
#    ln -s /usr/bin/pip3 /usr/bin/pip
#
#WORKDIR /root
#COPY . /root
#RUN pip install -U pip
#RUN pip install -r requirements.txt
#RUN pip install -r vdn_requirements.txt
#RUN pip install -r test/requirements.txt
#
#EXPOSE 3000
#
#ENV PYTHONPATH=$PYTHONPATH:/
