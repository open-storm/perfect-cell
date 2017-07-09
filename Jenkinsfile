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
    agent { label 'klab' }

    options {
        timeout(time: 30, unit: 'MINUTES')
    }

    environment {
        PSOC_VER = "4.1"
    }

    stages {
        stage('Build') {
            steps {
                //setBuildStatus("Building...", "PENDING");
                // Build the $BUILD version of the project.
                // Save to perfect-cell.cydsn\CortexM3\$ARCH\$BUILD\
                bat "${env.CYPRJMGR} -wrk \"${proj}.cydsn\\${proj}.cywrk\" -c ${build} -rebuild"
            }
        }
        stage('Program') {
            steps {
                //setBuildStatus("Programming...", "PENDING");
                bat "python build_tools\\psoc_program.py \"${proj}.cydsn\\CortexM3\\${arch}\\${build}\\${proj}.hex\""
            }
        }
        stage('Test') {
            steps {
                //setBuildStatus("Testing...", "PENDING");
                timeout(10) { // Only attempt for 10 minutes
                    waitUntil {
                        script {
                            def r = bat script: "python tests\\ci_test.py ${getCommitSHA()} \"${env.BUILD_TIMESTAMP}\"", returnStatus: true
                            return (r == 0)
                        }
                    }
                }
                // power off the psoc device after running the tests
                //bat "python build_tools\\psoc_program.py --power-off-device"
            }
        }
    }

    agent { label 'ec2' }
    post {
        always {
            sh "python3 tests/read_build_log.py \"${env.BUILD_TIMESTAMP}\""
            echo 'Build complete'
//            bat "\"C:\\Windows\\Sysnative\\bash.exe\" -c \"ssh -i ${env.SERVERKEY} ${env.SERVERADDR} python3 -u - < tests\\read_build_log.py '${env.BUILD_TIMESTAMP}'\""
        }
        /*
        success {
            setBuildStatus("Build complete", "SUCCESS");
        }

        failure {
            setBuildStatus("Build complete", "FAILURE");
        }
        */
    }
}

String getRepoURL() {
    bat "${env.GIT} config --get remote.origin.url > .git/remote-url"
    return readFile(".git/remote-url").trim()
}

String getCommitSHA() {
    bat "${env.GIT} rev-parse HEAD > .git/current-commit"
    return readFile(".git/current-commit").trim()
}

void setBuildStatus(String message, String state) {
    String repoURL = getRepoURL()

    step([
        $class: "GitHubCommitStatusSetter",
        reposSource: [$class: "ManuallyEnteredRepositorySource", url: repoURL],
        contextSource: [$class: "ManuallyEnteredCommitContextSource", context: "ci/jenkins/build-status"],
        errorHandlers: [[$class: "ChangingBuildStatusErrorHandler", result: "UNSTABLE"]],
        statusResultSource: [ $class: "ConditionalStatusResultSource", results: [[$class: "AnyBuildResult", message: message, state: state]] ]
    ]);
}
