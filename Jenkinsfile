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
                bash '''cmake -G Ninja'''
            }
        } 
        stage('Build') {
            steps {
                bash '''ninja'''
            }
        }
    }
}