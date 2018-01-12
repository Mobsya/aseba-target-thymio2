#!groovy

pipeline {
	agent none
	stages {
		stage('Prepare') {
			agent {
				label 'docker'
			}
			steps {
				// Jenkins will automatically check out the source
				sh 'git submodule update --init'
				sh '''
					rm -rf aseba
					git clone https://github.com/aseba-community/aseba.git --depth=1 --single-branch
				'''
				// Fixme: Stashed source includes .git otherwise submodule update fails
				stash includes: 'molole/**,cmake-microchip/**,aseba/**', name: 'externals'
			}
		}
		stage('Compile') {
			agent {
				dockerfile true
			}
			steps {
				unstash 'externals'
				// no dir('build') due to JENKINS-33510
				sh '''
					mkdir -p build && cd build
					cmake -DASEBA_DIR=$PWD/../aseba ..
					make
				'''
				stash includes: 'build/**', name: 'build'
			}
		}
		stage('Test') {
			agent { dockerfile true }
			steps {
				unstash 'externals'
				unstash 'build'
				// no dir('build') due to JENKINS-33510
				sh '''
					mkdir -p build && cd build
					LANG=C ctest
				'''
			}
		}
		stage('Archive') {
			agent any
			steps {
				unstash 'build'
				dir('build') {
					archiveArtifacts 'aseba-target-thymio2*'
				}
			}
		}
	}
}
