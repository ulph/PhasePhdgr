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
                sh '''cmake -G Ninja'''
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