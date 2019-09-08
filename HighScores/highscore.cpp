/*
# ----------------------------------------------
#   Name: Ricky Au, Johnas Wong
#   ID: 1529429, 1529241
#   CMPUT 275, Winter 2019
#
#   Final Project: Pacman: server
# ----------------------------------------------
*/

#include "serialport.h"

#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cassert>

using namespace std;

#define MAX_SCORES 100

bool recieveScore = false;
bool sendRanks = false;
bool sendAck = false;
bool newGame = false;

// struct that stores the names of players and the points they scored
struct Rankings{
  string names;
  unsigned int points;
};
Rankings playerRecord[MAX_SCORES];

int counter;

// function that lets the user input their username they want to store their score
string inputName(){
  cout << "Enter your username(no spaces): ";
  string playerName;
  cin >> playerName;
  return playerName;
}

// function that writes the string for the name and score to the text file score,txt
void writeText(string pName, string pScore){
  ofstream out;
  out.open("scores.txt", ios::app);
  out << pName;
  out << " ";
  out << pScore;
}

// function that takes a text file and reads it storing the names and points of previous highscores
void readScores(string filename, unordered_map<int,Rankings>& rank){
  ifstream fin(filename);
  string line;
  counter = 1;
  while (getline(fin, line)) {
    // since at most we will have 2 parameters
    string p[2];
    int at = 0;
    for (auto c : line) {
      // read in a space sperarated from text file
      if (c == ' ') {
        // start new string
        ++at;
      }
      else {
        // append character to the string we are building
        p[at] += c;
      }
      // don't want to index out of bounds!
      assert(at < 2);
    }
    // first value read from string is name
    playerRecord[counter].names = p[0];
    cout << "name: "<< p[0] << " ";
    // second value read from string is points
    playerRecord[counter].points = stoi(p[1]);
    cout << "points: " << p[1] << endl;
    counter++;
  }
}

// function that reads a score from the client and stores it in a struct and outputs into text file
int serverScore(string playerName, unordered_map<int,Rankings>& rank, int counter){
  SerialPort Serial("/dev/ttyACM0");
  string stats;
  string p[2];
  bool link = false;
  while (link == false) {
    do {
      stats = Serial.readline(10000);
    } while (stats.find("Score") == (string::npos));
    int at = 0;
    for (auto c: stats){
      if (c == ' '){
        at++;
      }
      else {
        p[at] += c;
      }
      assert(at < 2);
    }
    playerRecord[counter].names = playerName;
    playerRecord[counter].points = stoi(p[1]);
    counter = counter + 1;
    writeText(playerName, p[1]);
    link = true;
  }
  recieveScore = true;
  return counter;
}


// swap function that swaps two inputs
void swap(Rankings& place,Rankings& place2){
  Rankings temp = place;
  place = place2;
  place2 = temp;
}

/* This function takes last element as pivot, places
the pivot element at its correct position in sorted
	array, and places all smaller (smaller than pivot)
to left of pivot and all greater elements to right
of pivot */
int partition (Rankings pRec[], int low, int high){
	unsigned int pivot = pRec[high].points; // pivot
	int i = (low - 1); // Index of smaller element

	for (int j = low; j <= high- 1; j++)
	{
		// If current element is smaller than or
		// equal to pivot
		if (pRec[j].points >= pivot)
		{
      // increment index of smaller element
			i++;
			swap(pRec[i], pRec[j]);
		}
	}
	swap(pRec[i + 1], pRec[high]);
	return (i + 1);
}

// function that quicksorts and calls itself recursively
void quickSort(Rankings pRec[], int low, int high){
	if (low < high){
		/* pi is partitioning index, arr[p] is now
		at right place */
		int pi = partition(pRec, low, high);

		// Separately sort elements before
		// partition and after partition
		quickSort(pRec, low, pi - 1);
		quickSort(pRec, pi + 1, high);
	}
}

// function taht does the server communications
void serverCom (unordered_map<int,Rankings>& rank, int counter){
  SerialPort Serial("/dev/ttyACM0");
  string readIn;
  int sentCounter = 0;
  string stats;
  // while you havn't finished sending all scores
  while (sendRanks == false){
    // send the number of scores first
    if ((recieveScore == true) && (sendAck == false)){
      string countVal = to_string(counter);
      assert(Serial.writeline("N "));
      assert(Serial.writeline(countVal));
      assert(Serial.writeline("\n"));
      sendAck = true;
    // case to sendone name and one score everytime the client says its ready to recieve
    } else if ((recieveScore == true) && (sendAck == true)){
      do {
        readIn = Serial.readline(10000);
      } while (readIn.find("A") == (string::npos));
      // send up till the counter
      if (sentCounter < (counter - 1)){
        string val = to_string(playerRecord[sentCounter].points);
        assert(Serial.writeline("W "));
        assert(Serial.writeline(playerRecord[sentCounter].names));
        assert(Serial.writeline(" "));
        assert(Serial.writeline(val));
        assert(Serial.writeline("\n"));
        sentCounter++;
      }
      // send an E so client knows it is done
      if (sentCounter == (counter-1)){
        assert(Serial.writeline("E"));
        assert(Serial.writeline("\n"));
        sendRanks = true;
      }
    }
  }
}

// function that waits for the client to send if the user wants to reset game
void waiting(){
  SerialPort Serial("/dev/ttyACM0");
  string readIn;
  do {
    // timeout for readline increased to 100 seconds to reset on its own if user doesn't do anything
    readIn = Serial.readline(100000);
  } while (readIn.find("E") == (string::npos));
  newGame = true;
}

// function that is a template that will reset the struct
template<typename It>
void mfill(It begin, It end, typename std::remove_reference<decltype(*begin)>::type const &v)
{
   std::fill(begin, end, v);
}

int main(){
  SerialPort Serial("/dev/ttyACM0");
  unordered_map<int,Rankings> rank;
  // while loop to allow game to go on indefinately without recompiling
  while (true){
    newGame = false;
    recieveScore = false;
    sendRanks = false;
    sendAck = false;
    counter = 0;
    // reset the struct so no errors occur
    mfill(playerRecord, playerRecord+MAX_SCORES, {});
    // call the user to input their username
    string username = inputName();
    // read scores from text file
    readScores("scores.txt",rank);
    // store the scores of previous games and returns how many games their were
    counter = serverScore(username, rank, counter);
    // sort the struct by highsest score to lowest
    quickSort(playerRecord, 0, counter-1);
    // send sorted data to server
    serverCom(rank, counter);
    // wait for if the player wants a new game
    while (newGame == false){
      waiting();
    }
  }
  return 0;
}
