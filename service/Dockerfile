from ubuntu:18.04

run apt-get -qq update && apt-get install -qq --no-install-recommends xinetd rsyslog

# Note: anything that goes in public_files must be pre-built and checked in git
copy public/maybe /service
copy public/vault.bf /

# the deployment tester will pass in the flag from the yaml
ARG THE_FLAG="OOO{this is a test flag}"
RUN touch /flag && chmod 644 /flag && echo $THE_FLAG > /flag

copy service.conf /service.conf
copy banner_fail /banner_fail
copy wrapper /wrapper

expose 5000
cmd ["/usr/sbin/xinetd", "-syslog", "local0", "-dontfork", "-f", "/service.conf"]
