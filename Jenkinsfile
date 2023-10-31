@Library('xmos_jenkins_shared_library@v0.27.0') _

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
            defaultValue: '15.2.1',
            description: 'The XTC tools version'
        )
    }    
    environment {
        PYTHON_VERSION = "3.8.11"
        VENV_DIRNAME = ".venv"
        BUILD_DIRNAME = "dist"
        RTOS_TEST_RIG_TARGET = "xcore_sdk_test_rig"
        LOCAL_WIFI_SSID = credentials('hampton-office-network-ssid')
        LOCAL_WIFI_PASS = credentials('hampton-office-network-wifi-password')
    }    
    stages {
        stage('Build and Docs') {
            parallel {
                stage('Build Docs') {
                    agent { label "docker" }
                    environment { XMOSDOC_VERSION = "v4.0" }
                    steps {
                        checkout scm
                        sh 'git submodule update --init --recursive --depth 1'
                        sh "docker pull ghcr.io/xmos/xmosdoc:$XMOSDOC_VERSION"
                        sh """docker run -u "\$(id -u):\$(id -g)" \
                            --rm \
                            -v ${WORKSPACE}:/build \
                            ghcr.io/xmos/xmosdoc:$XMOSDOC_VERSION -v"""
                        archiveArtifacts artifacts: "doc/_build/**", allowEmptyArchive: true
                    }
                    post {
                        cleanup {
                            xcoreCleanSandbox()
                        }
                    }
                }

                stage('Build and Test') {
                    when {
                        expression { !env.GH_LABEL_DOC_ONLY.toBoolean() }
                    }
                    agent {
                        label 'xcore.ai-explorer-us'
                    }
                    stages {
                        stage('Checkout') {
                            steps {
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
                                            uid = sh(returnStdout: true, script: 'id -u').trim()
                                            gid = sh(returnStdout: true, script: 'id -g').trim()
                                            withXTAG(["$RTOS_TEST_RIG_TARGET"]) { adapterIDs ->
                                                sh "docker run --rm -u $uid:$gid --privileged -v /dev:/dev -w /fwk_rtos -v $WORKSPACE:/fwk_rtos ghcr.io/xmos/xcore_voice_tester:develop bash -l test/rtos_drivers/usb/check_usb.sh " + adapterIDs[0]
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
