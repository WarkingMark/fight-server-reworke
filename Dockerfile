## ЭТАП 1: СБОРКА
#FROM centos:7 AS builder
#
#RUN cd /etc/yum.repos.d/ && \
#    sed -i 's/mirrorlist/#mirrorlist/g' /etc/yum.repos.d/CentOS-* && \
#    sed -i 's|#baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g' /etc/yum.repos.d/CentOS-*
#
#RUN yum install -y epel-release && \
#    cd /etc/yum.repos.d/ && \
#    sed -i 's/mirrorlist/#mirrorlist/g' /etc/yum.repos.d/epel* && \
#    sed -i 's|#baseurl=http://download.fedoraproject.org/pub|baseurl=http://archives.fedoraproject.org/pub|g' /etc/yum.repos.d/epel*
#
## Устанавливаем зависимости
#RUN yum install -y \
#    gcc \
#    make \
#    lua-devel \
#    tolua++-devel \
#    && yum clean all
#
## Создаём симлинки для библиотек (если нужно)
#RUN ln -sf /usr/lib64/liblua-5.1.so /usr/lib64/liblua5.1.so && \
#    ln -sf /usr/lib64/libtolua++.so /usr/lib64/libtolua++5.1.so
#
#WORKDIR /usr/src/app
#COPY . /usr/src/app/
#
## Исправляем Makefile на лету (меняем имена библиотек)
#RUN sed -i 's/-llua5.1/-llua-5.1/g' Makefile && \
#    sed -i 's/-ltolua++5.1/-ltolua++/g' Makefile
#
#RUN make fightd
#
## ЭТАП 2: ЗАПУСК
#FROM centos:7
#
#RUN cd /etc/yum.repos.d/ && \
#    sed -i 's/mirrorlist/#mirrorlist/g' /etc/yum.repos.d/CentOS-* && \
#    sed -i 's|#baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g' /etc/yum.repos.d/CentOS-*
#
#RUN yum install -y epel-release && \
#    cd /etc/yum.repos.d/ && \
#    sed -i 's/mirrorlist/#mirrorlist/g' /etc/yum.repos.d/epel* && \
#    sed -i 's|#baseurl=http://download.fedoraproject.org/pub|baseurl=http://archives.fedoraproject.org/pub|g' /etc/yum.repos.d/epel*
#
#RUN yum install -y lua && yum clean all
#RUN ln -sf /usr/lib64/liblua-5.1.so /usr/lib64/liblua5.1.so.0 && ldconfig
#
#WORKDIR /app
#COPY --from=builder /usr/src/app/fightd .
#RUN chmod +x ./fightd

FROM centos:7

# Чиним репозитории
RUN cd /etc/yum.repos.d/ && \
    sed -i 's/mirrorlist/#mirrorlist/g' /etc/yum.repos.d/CentOS-* && \
    sed -i 's|#baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g' /etc/yum.repos.d/CentOS-*

RUN yum install -y epel-release && \
    cd /etc/yum.repos.d/ && \
    sed -i 's/mirrorlist/#mirrorlist/g' /etc/yum.repos.d/epel* && \
    sed -i 's|#baseurl=http://download.fedoraproject.org/pub|baseurl=http://archives.fedoraproject.org/pub|g' /etc/yum.repos.d/epel*

# Устанавливаем зависимости для сборки и запуска
RUN yum install -y \
    gcc \
    make \
    lua-devel \
    tolua++-devel \
    lua \
    && yum clean all

# Создаём симлинки для библиотек
RUN ln -sf /usr/lib64/liblua-5.1.so /usr/lib64/liblua5.1.so && \
    ln -sf /usr/lib64/liblua-5.1.so /usr/lib64/liblua5.1.so.0 && \
    ln -sf /usr/lib64/libtolua++.so /usr/lib64/libtolua++5.1.so && \
    ldconfig

WORKDIR /app

CMD ["/bin/bash"]