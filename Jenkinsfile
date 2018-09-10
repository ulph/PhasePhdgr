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
                cmake -G Ninja
                ninja
            }
        }
    }
}