#! /usr/bin/env groovy

/*
 * This Jenkinsfile is intended to run on our internal server and may fail
 * anywhere else.
 * It makes assumptions about plugins being installed, labels mapping to nodes
 * that can build what is needed, etc.
 */

String proj = "perfect-cell"
String build = "Debug" // Debug | Release, debug build enables testing.
String arch = "ARM_GCC_541"

pipeline {
    agent none

    options {
        timeout(time: 30, unit: 'MINUTES')
    }

    environment {
        PSOC_VER = "4.1"
    }

    stages {
        stage('Build') {
        agent { label 'klab' }
            steps {
                // Build the $BUILD version of the project.
                // Save to perfect-cell.cydsn\CortexM3\$ARCH\$BUILD\
                bat "${env.CYPRJMGR} -wrk \"${proj}.cydsn\\${proj}.cywrk\" -c ${build} -rebuild"
            }
        }
        stage('Program') {
        agent { label 'klab' }
            steps {
                bat "python build_tools\\psoc_program.py \"${proj}.cydsn\\CortexM3\\${arch}\\${build}\\${proj}.hex\""
            }
        }
        stage('Test') {
        // TODO: These steps could probably be run on the EC2 instance
        agent { label 'klab' }
            steps {
                bat "python build_tools\\pre_build.py"
                timeout(10) { // Only attempt for 10 minutes
                    waitUntil {
                        script {
                            def r = bat script: "python tests\\ci_test.py ${getCommitSHA()} \"${env.BUILD_TIMESTAMP}\"", returnStatus: true
                            if (r != 0) { sleep 30 }
                            return (r == 0)
                        }
                    }
                }
                // power off the psoc device after running the tests
                //bat "python build_tools\\psoc_program.py --power-off-device"
            }
        }
    }

    post {
        success {
            node('master') {
                checkout scm
                sh "python3 ./build_tools/post_build.py \"SUCCESS\""
            }
        }
        unstable {
            node('master') {
                checkout scm
                sh "python3 ./build_tools/post_build.py \"UNSTABLE\""
            }
        }
        failure {
            node('master') {
                checkout scm
                sh "python3 ./build_tools/post_build.py \"FAILURE\""
            }
        }
        notBuilt {
            node('master') {
                checkout scm
                sh "python3 ./build_tools/post_build.py \"NOT_BUILT\""
            }
        }
        aborted {
            node('master') {
                checkout scm
                sh "python3 ./build_tools/post_build.py \"ABORTED\""
            }
        }
        always {
            node('klab') {
                deleteDir() // clean up our workspace on the slave
            }
            node('master') {
                checkout scm
                sh "python3 tests/read_build_log.py \"${env.BUILD_TIMESTAMP}\""
                //sh "echo ${getBuildResult()}"
                deleteDir() // clean up our workspace on master
            }
        }
    }
}

String getBuildResult() {
    return "${buildStatus}"
}

String getCommitSHA() {
    bat "${env.GIT} rev-parse HEAD > .git/current-commit"
    return readFile(".git/current-commit").trim()
}