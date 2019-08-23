FROM ubuntu:14.04

RUN apt-get update && apt-get install -y

RUN apt-get install -y \
	build-essential \
	libxml2-dev \
	libxslt1-dev \
	libcroco3-dev \
	librsvg2-dev \
	libjpeg-dev  \
	libgif-dev \
	libssl-dev \
	libreadline6 \
	libreadline6-dev \
	libffi-dev \
	libgmp-dev \
	zlib1g-dev \
	libyaml-dev  \
	libsqlite3-dev \
	libxslt-dev \
	libc6-dev \
	ncurses-dev \
	libgdbm-dev \
	libgdbm-dev \
	libvlc-dev \
	libgtk-3-dev \
	gawk \
	vlc \
	curl \
	ca-certificates \
	gnupg2 \
	openssl \
	zlib1g \
	autoconf \
	automake \
	libtool \
	bison \
	pkg-config \
	git \
	sqlite3

# download & compile ruby:
# RUN curl -O http://cache.ruby-lang.org/pub/ruby/2.3/ruby-2.3.7.tar.bz2 > ruby-2.3.7.tar.bz2
# RUN tar xvfj ruby-2.3.7.tar.bz2
# RUN cd ruby-2.3.7 &&./configure --enable-load-relative && make -j6 && sudo make install -j6

RUN adduser --disabled-password --gecos '' zach
RUN adduser zach sudo
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
USER zach 
ENV HOME /home/zach

RUN gpg2 --keyserver hkp://pool.sks-keyservers.net --recv-keys 409B6B1796C275462A1703113804BB82D39DC0E3 7D2BAF1CF37B13E2069D6956105BD0E739499BDB
RUN curl -sSL https://get.rvm.io | bash -s stable
RUN /bin/bash -c "source $HOME/.rvm/scripts/rvm && rvm install 2.3.7 -C --enable-load-relative"

WORKDIR $HOME/shoes3

RUN /bin/bash -c "source $HOME/.rvm/scripts/rvm && gem install rake"
RUN /bin/bash -c "source $HOME/.rvm/scripts/rvm && gem install ffi"
RUN /bin/bash -c "source $HOME/.rvm/scripts/rvm && gem install sqlite3"
RUN /bin/bash -c "source $HOME/.rvm/scripts/rvm && gem install nokogiri"
RUN /bin/bash -c "source $HOME/.rvm/scripts/rvm && gem install chipmunk"
RUN /bin/bash -c "source $HOME/.rvm/scripts/rvm && gem install yajl-ruby"
# RUN gem install picky --no-ri --no-rdoc

COPY . .

RUN /bin/bash -c "source $HOME/.rvm/scripts/rvm && rake setup:minlin"

CMD ["minlin/shoes"]
