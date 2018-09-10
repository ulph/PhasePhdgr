pipeline {
    agent any

    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }    
        stage('Configure / Generate') {
            steps {
                sh '''cmake -G Ninja --DBUILD_GTEST=1 --DBUILD_JUCE=1 --DBUILD_CLI=1 --DSUPPORT_PLUGIN_LOADING=1 --DBUILD_PLUGIN_SDK=1'''
            }
        } 
        stage('Build') {
            steps {
                sh '''ninja'''
            }
        }
        stage('Test') {
            steps {
                sh '''ctest'''
            }
        }
    }
}