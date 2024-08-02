@Library('xmos_jenkins_shared_library@v0.33.0') _

def runningOn(machine) {
  println "Stage running on:"
  println machine
}

def checkSkipLink() {
    def skip_linkcheck = ""
    if (env.GH_LABEL_ALL.contains("skip_linkcheck")) {
        println "skip_linkcheck set, skipping link check..."
        skip_linkcheck = "clean html pdf"
    }
    return skip_linkcheck
}

def buildDocs(String zipFileName) {
  withVenv {
    sh 'pip install git+ssh://git@github.com/xmos/xmosdoc@v5.2.0'
    sh "xmosdoc ${checkSkipLink()}"
    zip zipFile: zipFileName, archive: true, dir: "doc/_build"
  }
}

getApproval()

pipeline {
    agent none
    options {
        disableConcurrentBuilds()
        skipDefaultCheckout()
        timestamps()
        // on develop discard builds after a certain number else keep forever
        buildDiscarder(logRotator(
            numToKeepStr:         env.BRANCH_NAME ==~ /develop/ ? '25' : '',
            artifactNumToKeepStr: env.BRANCH_NAME ==~ /develop/ ? '25' : ''
        ))
    }
    parameters {
        string(
            name: 'TOOLS_VERSION',
            // Dropped back from 15.3.0 on 2nd August 2024 because of http://bugzilla.xmos.local/show_bug.cgi?id=18895
            defaultValue: '15.2.1',
            description: 'The XTC tools version'
        )
    }
    environment {
        PYTHON_VERSION = "3.8.11"
        VENV_DIRNAME = ".venv"
        BUILD_DIRNAME = "dist"
        RTOS_TEST_RIG_TARGET = "XCORE-AI-EXPLORER"
        LOCAL_WIFI_SSID = credentials('bristol-office-development-wifi-ssid')
        LOCAL_WIFI_PASS = credentials('bristol-office-development-wifi-password')
    }
    stages {
        stage('Build and Docs') {
            parallel {
                stage('Build Docs') {
                    agent { label "documentation" }
                    steps {
                        runningOn(env.NODE_NAME)
                        dir('fwk_rtos'){
                            checkout scm
                            createVenv()
                            withTools(params.TOOLS_VERSION) {
                                buildDocs("fwk_rtos_docs.zip")
                            } // withTools
                        } // dir
                    } // steps
                    post {
                        cleanup {
                            xcoreCleanSandbox()
                        }
                    } 
                } // Build Docs

                stage('Build and Test') {
                    when {
                        expression { !env.GH_LABEL_DOC_ONLY.toBoolean() }
                    }
                    agent {
                        label 'xcore.ai-explorer-hil-tests'
                    }
                    stages {
                        stage('Checkout') {
                            steps {
                                runningOn(env.NODE_NAME)
                                checkout scm
                                sh 'git submodule update --init --recursive --depth 1 --jobs \$(nproc)'
                            }
                        }
                        stage('Build tests and host apps') {
                            steps {
                                script {
                                    uid = sh(returnStdout: true, script: 'id -u').trim()
                                    gid = sh(returnStdout: true, script: 'id -g').trim()
                                }
                                // pull docker
                                sh "docker pull ghcr.io/xmos/xcore_voice_tester:develop"
                                withTools(params.TOOLS_VERSION) {
                                    sh "bash tools/ci/build_rtos_tests.sh"
                                    sh "bash tools/ci/build_host_apps.sh"
                                }
                                // List built files for log
                                sh "ls -la dist/"
                                sh "ls -la dist_host/"
                            }
                        }
                        stage('Create virtual environment') {
                            steps {
                                // Create venv
                                sh "pyenv install -s $PYTHON_VERSION"
                                sh "~/.pyenv/versions/$PYTHON_VERSION/bin/python -m venv $VENV_DIRNAME"
                                // Install dependencies
                                withVenv() {
                                    sh "pip install git+https://github0.xmos.com/xmos-int/xtagctl.git"
                                    sh "pip install -r test/requirements.txt"
                                }
                            }
                        }
                        stage('Cleanup xtagctl') {
                            steps {
                                // Cleanup any xtagctl cruft from previous failed runs
                                withTools(params.TOOLS_VERSION) {
                                    withVenv {
                                        sh "xtagctl reset_all $RTOS_TEST_RIG_TARGET"
                                    }
                                }
                                sh "rm -f ~/.xtag/status.lock ~/.xtag/acquired"
                            }
                        }
                        stage('Run RTOS Drivers WiFi test') {
                           steps {
                               withTools(params.TOOLS_VERSION) {
                                   withVenv {
                                       script {
                                           withXTAG(["$RTOS_TEST_RIG_TARGET"]) { adapterIDs ->
                                               sh "test/rtos_drivers/wifi/check_wifi.sh " + adapterIDs[0]
                                           }
                                           sh "pytest test/rtos_drivers/wifi"
                                       }
                                   }
                               }
                           }
                        }
                        stage('Run RTOS Drivers HIL test') {
                            steps {
                                withTools(params.TOOLS_VERSION) {
                                    withVenv {
                                        script {
                                            withXTAG(["$RTOS_TEST_RIG_TARGET"]) { adapterIDs ->
                                                sh "test/rtos_drivers/hil/check_drivers_hil.sh " + adapterIDs[0]
                                            }
                                            sh "pytest test/rtos_drivers/hil"
                                        }
                                    }
                                }
                            }
                        }
                        stage('Run RTOS Drivers HIL_Add test') {
                            steps {
                                withTools(params.TOOLS_VERSION) {
                                    withVenv {
                                        script {
                                            withXTAG(["$RTOS_TEST_RIG_TARGET"]) { adapterIDs ->
                                                sh "test/rtos_drivers/hil_add/check_drivers_hil_add.sh " + adapterIDs[0]
                                            }
                                            sh "pytest test/rtos_drivers/hil_add"
                                        }
                                    }
                                }
                            }
                        }
                        stage('Run RTOS Drivers USB test') {
                            steps {
                                withTools(params.TOOLS_VERSION) {
                                    withVenv {
                                        script {
                                            withXTAG(["$RTOS_TEST_RIG_TARGET"]) { adapterIDs ->
                                                sh "bash -l test/rtos_drivers/usb/check_usb.sh " + adapterIDs[0]
                                            }
                                            sh "pytest test/rtos_drivers/usb"
                                        }
                                    }
                                }
                            }
                        }
                    }
                    post {
                        cleanup {
                            // cleanWs removes all output and artifacts of the Jenkins pipeline
                            //   Comment out this post section to leave the workspace which can be useful for running items on the Jenkins agent.
                            //   However, beware that this pipeline will not run if the workspace is not manually cleaned.
                            xcoreCleanSandbox()
                        }
                    }
                }
            }
        }
    }
}
