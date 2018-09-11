pipeline {
    agent any

    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }    
        stage('Configure') {
            steps {
                sh '''cmake -G Ninja -DBUILD_JUCE=ON -DBUILD_CLI=ON -DSUPPORT_PLUGIN_LOADING=ON -DBUILD_PLUGIN_SDK=ON .'''
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
        stage('Package') {
            steps {
                echo '''NYI'''
            }
        }
        stage('Publish') {
            steps {
                echo '''NYI'''
            }
        }
    }
}