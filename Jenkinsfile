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
                sh '''cmake -G Ninja -DBUILD_GTEST=ON -DBUILD_JUCE=ON -DBUILD_CLI=ON -DSUPPORT_PLUGIN_LOADING=ON -DBUILD_PLUGIN_SDK=ON .'''
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