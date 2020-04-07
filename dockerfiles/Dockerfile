FROM ubuntu:16.04

RUN sed -i "s/archive.ubuntu.com/mirrors.aliyun.com/" /etc/apt/sources.list \
    && sed -i "s/security.ubuntu.com/mirrors.aliyun.com/" /etc/apt/sources.list \
    && apt-get update \
    && apt-get upgrade -y python3 \
    && apt-get install -y build-essential python3-dev git wget curl \
        make build-essential libssl-dev zlib1g-dev libbz2-dev libreadline-dev libsqlite3-dev wget curl llvm libncurses5-dev xz-utils tk-dev libxml2-dev libxmlsec1-dev libffi-dev liblzma-dev \
    && git clone https://github.com/pyenv/pyenv.git ~/.pyenv \
    && echo 'export PYENV_ROOT="$HOME/.pyenv"' >> /root/.bashrc \
    && echo 'export PATH="$PYENV_ROOT/bin:$PATH"' >> ~/.bashrc \
    && echo 'if command -v pyenv 1>/dev/null 2>&1; then\n  eval "$(pyenv init -)"\nfi' >> ~/.bashrc

WORKDIR /tmp
