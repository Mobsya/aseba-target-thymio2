# Customize the official Jenkins container with the MPLABX XC16 compiler
# David Sherman 2018-01-08
#
# Build using
#     docker build docker -t aseba/mplabx .
# For an interactive shell, run using
#     docker run -it -w /var/jenkins_home --entrypoint /bin/bash aseba/mplabx

FROM jenkins

USER root

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    curl \
    gcc-multilib \
    git \
    sudo \
    && rm -rf /var/lib/apt/lists/*

# Allow sudo by user 'jenkins' without password
RUN gpasswd -a jenkins sudo && \
    sed -i.bkp -e \
      's/%sudo\s\+ALL=(ALL\(:ALL\)\?)\s\+ALL/%sudo ALL=NOPASSWD:ALL/g' \
      /etc/sudoers

# Retrieve XC16 compiler and install headless
RUN curl -fSL -A 'Mozilla/5.0' -o /tmp/xc16-install.run \
        'http://www.microchip.com/mplabxc16linux' && \
    chmod a+x /tmp/xc16-install.run && \
    /tmp/xc16-install.run --prefix /opt/microchip/xc16 \
        --mode unattended --unattendedmodeui none \
        --netservername localhost --LicenseType FreeMode && \
    rm -f /tmp/xc16-install.run

# Retrieve PIC24/dsPIC Peripheral Libraries and install headless
RUN curl -fSL -A 'Mozilla/5.0' -o /tmp/plib16-install.run \
    'http://ww1.microchip.com/downloads/en/DeviceDoc/peripheral-libraries-for-pic24-and-dspic-v2.00-linux-installer.run' && \
    chmod a+x /tmp/plib16-install.run && \
    /tmp/plib16-install.run --prefix /opt/microchip/xc16 \
        --mode unattended --unattendedmodeui none && \
    rm -f /tmp/plib16-install.run

# Add MPLABX to default PATH
RUN echo 'export PATH=/opt/microchip/xc16/bin:${PATH}' >> /etc/profile.d/mplabx.sh

# Switch to user 'jenkins'
USER jenkins
