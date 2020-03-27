#-------------------------------------------------------------------------------------------------------------
# Licensed under the MIT License.
# Based on https://github.com/ashutoshkumr/vscode-cpp-dev-env
#-------------------------------------------------------------------------------------------------------------

FROM olned/librdkafka

WORKDIR /app

ARG USERNAME=olned
ARG USER_UID=1000
ARG USER_GID=$USER_UID

ENV DEBIAN_FRONTEND=noninteractive
ENV LOG_DIR=/workspaces/learn-kafka/log

RUN apt-get update \
    && apt-get -y install --no-install-recommends apt-utils dialog 2>&1 \
    # Verify git, process tools, lsb-release (useful for CLI installs) installed
    && apt-get -y install git iproute2 procps lsb-release \
    # Install build-essential cmake
    && apt-get -y install build-essential cmake \
    # Install C++ tools
    && apt-get -y install cppcheck valgrind gdb

    # Create a non-root user to use if preferred - see https://aka.ms/vscode-remote/containers/non-root-user.
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd -s /bin/bash --uid $USER_UID --gid $USER_GID -m $USERNAME \
    # [Optional] Add sudo support for the non-root user
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME\
    && chmod 0440 /etc/sudoers.d/$USERNAME

# Switch back to dialog for any ad-hoc use of apt-get
ENV DEBIAN_FRONTEND=dialog

