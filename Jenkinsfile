pipeline {
    agent any

    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }    
        stage('Build') {
            steps {
                bash '''cmake -G Ninja'''
                bash '''ninja'''
            }
        }
    }
}