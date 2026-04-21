#include <iostream>
#include <fstream>

#include <string>
#include <vector>
#include <set>
#include <map>

#include "person.h"

using namespace std;

// Function prototypes

void readFileWordsIntoVector(vector<string> &vec, string filename);
void vocabWordLengthDistributionCount(map<int, int>& count, const vector<string> &vocab);
void vocabWordCharDistributionCount(map<char, int>& count, const vector<string>& vocab);

double prCharGivenCharOfState(char charGenerated, char charOfTheState);

void getPrTableForPossibleInitialStates(double prTable[], int sizeOfTable);
void getPrTableForPossibleNextStates(double prTable[], int sizeOfTable, int currentState);

int take1SampleFrom1PrSpace(double prTable[], int sizeOfTable);
void getKeyboardProbabilityTable(char charToType, double prTable[]);

char typeOneChar(char charToType);
string typeOneWord(string word);

double prOf1CharSeriesWhenTypingOneWord(string observed, string original);
void getProbabilitySumGivenCorruptedWord(vector<pair<double, int>>& output, vector<string> vocab, string wordGenerated, int id);

void sort(vector<pair<double, int>>& vec);

// Parameter values used in the functions
double testPHit, testPMiss, testPRepeat, testPMoveOn, testDegSp, testDegKb;

int main()
{
	cout << "Begin demo of author recognition:" << endl << endl;

	/**************************************************************************************************/
	// STEP 1: Get vocabulary information

	vector<string> vocabWords;
	readFileWordsIntoVector(vocabWords, "jumps.txt"); // Get unique words

	if (vocabWords.empty())
	{
		cout << "Vocabulary list is empty!" << endl;
		return 1;
	}

	// Get word length and character distribution

	map<int, int> lengthDistribution;
	map<char, int> charDistribution;

	vocabWordLengthDistributionCount(lengthDistribution, vocabWords);
	vocabWordCharDistributionCount(charDistribution, vocabWords);

	// Display results
	cout << "Vocabulary information:" << endl << endl;

	cout << "Number of unique words: " << vocabWords.size() << endl << endl;

	cout << "Vocab word length distribution:" << endl;
	for (auto l = lengthDistribution.begin(); l != lengthDistribution.end(); l++)
	{
		cout << l->first << " : " << l->second << "\t: ";
		for (int i = 0; i < l->second; i++)
			cout << "*";
		cout << endl;
	}
	cout << endl;

	cout << "Vocab list character distribution:" << endl;
	for (auto c = charDistribution.begin(); c != charDistribution.end(); c++)
	{
		cout << c->first << " : " << c->second << "\t: ";
		for (int i = 0; i < c->second; i++)
			cout << "*";
		cout << endl;
	}
	cout << endl;

	/**************************************************************************************************/
	// STEP 2: Store the possible parameter values

	vector<double> pHit = { 0.3, 0.6, 0.9 };
	vector<double> degKb = { 1.5, 2, 3 };

	vector<double> pRepeat = { 0.1, 0.4, 0.7 };
	vector<double> degSp = { 2, 3, 4 };

	vector<Person> possibleAuthors;
	int x = 0;

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			for (int k = 0; k < 3; k++)
				for (int l = 0; l < 3; l++)
				{
					// Create an author (instance of class "Person") and append it to "possibleAuthors"
					Person p(pRepeat[i], pHit[j], degSp[k], degKb[l], x);
					possibleAuthors.push_back(p);
					x++;
				}
	cout << endl;

	/**************************************************************************************************/
	// STEP 3: Perform author recognition for all 81 authors

	vector<int> predictionDistribution;
	for (int i = 0; i < 81; i++)
		predictionDistribution.push_back(0);

	int cumulativeTopTen = 0;
	int cumulativeTopFive = 0;
	int cumulativeNumCorrect = 0;

	vector<int> cumulativePredictionDistribution;
	for (int i = 0; i < 81; i++)
		cumulativePredictionDistribution.push_back(0);

	int id = 0;
	for (auto dk : degKb)
		for (auto ds : degSp)
			for (double pr : pRepeat)
				for (auto ph : pHit) // Loop through all parameter combinations
				{
					testDegKb = dk;
					testDegSp = ds;
					testPRepeat = pr;
					testPMoveOn = 1 - pr;
					testPHit = ph;
					testPMiss = 1 - ph;

					// Store the parameter values for the true author, Person X, with their id
					Person trueAuthor(testPRepeat, testPHit, testDegSp, testDegKb, id);
					id++;

					int numCorrectPredictions = 0;
					int numInTopFive = 0;
					int numInTopTen = 0;

					vector<double> manhattanDistance;
					vector<pair<double, int>> cumulativeProbabilitySums;

					for (int i = 0; i < 81; i++)
						cumulativeProbabilitySums.push_back(make_pair(0, i));

					// Have Person X type 50 random words from the vocabulary list
					for (int i = 0; i < 50; i++)
					{
						string wordToType;
						string wordGenerated;

						// Get a random word to type and generate a corrupted version based on Person 0's parameter values
						wordToType = vocabWords[rand() % vocabWords.size()];
						wordGenerated = typeOneWord(wordToType);

						// Find the sum of the probability that typed word X was originally vocab word Y, given a set of parameters

						vector<pair<double, int>> probabilitySums;
						int id2 = 0;

						for (auto dk2 : degKb)
							for (auto ds2 : degSp)
								for (auto pr2 : pRepeat)
									for (auto ph2 : pHit)
									{
										// Set the global test parameters
										testPHit = ph2;
										testPMiss = 1 - testPHit;

										testPRepeat = pr2;
										testPMoveOn = 1 - testPRepeat;

										testDegSp = ds2;
										testDegKb = dk2;

										// Get the sum
										getProbabilitySumGivenCorruptedWord(probabilitySums, vocabWords, wordGenerated, id2);
										id2++;
									}

						// Add the corresponding probability sums to the cumulative sums list
						for (int i = 0; i < probabilitySums.size(); i++)
							cumulativeProbabilitySums[i].first += probabilitySums[i].first;

						// Sort the probability sums
						sort(probabilitySums);

						// Get the top 10 most likely authors
						int idMostLikely = probabilitySums[0].second;
						int idSecond = probabilitySums[1].second;
						int idThird = probabilitySums[2].second;
						int idFourth = probabilitySums[3].second;
						int idFifth = probabilitySums[4].second;
						int idSixth = probabilitySums[5].second;
						int idSeventh = probabilitySums[6].second;
						int idEighth = probabilitySums[7].second;
						int idNinth = probabilitySums[8].second;
						int idTenth = probabilitySums[9].second;

						// Update the prediction distribution
						predictionDistribution[idMostLikely]++;
						predictionDistribution[idSecond]++;
						predictionDistribution[idThird]++;
						predictionDistribution[idFourth]++;
						predictionDistribution[idFifth]++;
						predictionDistribution[idSixth]++;
						predictionDistribution[idSeventh]++;
						predictionDistribution[idEighth]++;
						predictionDistribution[idNinth]++;
						predictionDistribution[idTenth]++;

						// Update the num in top 10, num in top 5, and num most likely
						if (possibleAuthors[idMostLikely].getName() == trueAuthor.getName() || possibleAuthors[idSecond].getName() == trueAuthor.getName() || possibleAuthors[idThird].getName() == trueAuthor.getName() || possibleAuthors[idFourth].getName() == trueAuthor.getName() || possibleAuthors[idFifth].getName() == trueAuthor.getName()
							|| possibleAuthors[idSixth].getName() == trueAuthor.getName() || possibleAuthors[idSeventh].getName() == trueAuthor.getName() || possibleAuthors[idEighth].getName() == trueAuthor.getName() || possibleAuthors[idNinth].getName() == trueAuthor.getName() || possibleAuthors[idTenth].getName() == trueAuthor.getName())
						{
							numInTopTen++;

							if (possibleAuthors[idMostLikely].getName() == trueAuthor.getName() || possibleAuthors[idSecond].getName() == trueAuthor.getName() || possibleAuthors[idThird].getName() == trueAuthor.getName() || possibleAuthors[idFourth].getName() == trueAuthor.getName() || possibleAuthors[idFifth].getName() == trueAuthor.getName())
								numInTopFive++;

							if (possibleAuthors[idMostLikely].getName() == trueAuthor.getName())
								numCorrectPredictions++;
						}

						// Get the Manhattan distance between the most likely author and the true author
						double distanceRepeat = abs(possibleAuthors[idMostLikely].getPrRepeat() - trueAuthor.getPrRepeat());
						double distanceHit = abs(possibleAuthors[idMostLikely].getPrHit() - trueAuthor.getPrHit());
						double distanceDegSp = abs(possibleAuthors[idMostLikely].getDegSp() - trueAuthor.getDegSp());
						double distanceDegKb = abs(possibleAuthors[idMostLikely].getDegKb() - trueAuthor.getDegKb());

						double distance = distanceRepeat + distanceHit + distanceDegSp + distanceDegKb;
						manhattanDistance.push_back(distance);
					}

					// Get percents
					double percentCorrectPredictions = (double)numCorrectPredictions / (double)50 * (double)100;
					double percentInTop5 = (double)numInTopFive / (double)50 * (double)100;
					double percentInTop10 = (double)numInTopTen / (double)50 * (double)100;

					// Get average manhattan distance
					double distanceSum = 0;
					for (double distance : manhattanDistance)
						distanceSum += distance;
					double avgManhattanDistance = distanceSum / manhattanDistance.size();

					// Get variance on manhattan distance
					distanceSum = 0;
					for (double distance : manhattanDistance)
					{
						double diffSquared = pow(distance - avgManhattanDistance, 2);
						distanceSum += diffSquared;
					}
					double variance = distanceSum / (manhattanDistance.size() - 1);

					// Sort the cumulative sums
					sort(cumulativeProbabilitySums);

					// Get greater top 10
					int idMostLikely = cumulativeProbabilitySums[0].second;
					int idSecond = cumulativeProbabilitySums[1].second;
					int idThird = cumulativeProbabilitySums[2].second;
					int idFourth = cumulativeProbabilitySums[3].second;
					int idFifth = cumulativeProbabilitySums[4].second;
					int idSixth = cumulativeProbabilitySums[5].second;
					int idSeventh = cumulativeProbabilitySums[6].second;
					int idEighth = cumulativeProbabilitySums[7].second;
					int idNinth = cumulativeProbabilitySums[8].second;
					int idTenth = cumulativeProbabilitySums[9].second;

					// Update the distribution
					cumulativePredictionDistribution[idMostLikely]++;
					cumulativePredictionDistribution[idSecond]++;
					cumulativePredictionDistribution[idThird]++;
					cumulativePredictionDistribution[idFourth]++;
					cumulativePredictionDistribution[idFifth]++;
					cumulativePredictionDistribution[idSixth]++;
					cumulativePredictionDistribution[idSeventh]++;
					cumulativePredictionDistribution[idEighth]++;
					cumulativePredictionDistribution[idNinth]++;
					cumulativePredictionDistribution[idTenth]++;


					// Update the cumulative num in top 10, num in top 5, and num most likely
					if (possibleAuthors[idMostLikely].getName() == trueAuthor.getName() || possibleAuthors[idSecond].getName() == trueAuthor.getName() || possibleAuthors[idThird].getName() == trueAuthor.getName() || possibleAuthors[idFourth].getName() == trueAuthor.getName() || possibleAuthors[idFifth].getName() == trueAuthor.getName()
						|| possibleAuthors[idSixth].getName() == trueAuthor.getName() || possibleAuthors[idSeventh].getName() == trueAuthor.getName() || possibleAuthors[idEighth].getName() == trueAuthor.getName() || possibleAuthors[idNinth].getName() == trueAuthor.getName() || possibleAuthors[idTenth].getName() == trueAuthor.getName())
					{
						cumulativeTopTen++;

						if (possibleAuthors[idMostLikely].getName() == trueAuthor.getName() || possibleAuthors[idSecond].getName() == trueAuthor.getName() || possibleAuthors[idThird].getName() == trueAuthor.getName() || possibleAuthors[idFourth].getName() == trueAuthor.getName() || possibleAuthors[idFifth].getName() == trueAuthor.getName())
							cumulativeTopFive++;

						if (possibleAuthors[idMostLikely].getName() == trueAuthor.getName())
							cumulativeNumCorrect++;
					}

					// Display the results

					cout << "FINAL RESULTS for Real Author " << trueAuthor.getName() << ":" << endl << endl
						<< "deg_obs = " << trueAuthor.getDegKb() << endl
						<< "deg_tran = " << trueAuthor.getDegSp() << endl
						<< "p_iter = " << trueAuthor.getPrRepeat() << endl
						<< "p_mode = " << trueAuthor.getPrHit() << endl << endl;

					cout << "% of times the true author appeared in the top 1: " << percentCorrectPredictions << endl
						<< "% ... in the top 5: " << percentInTop5 << endl
						<< "% ... in the top 10: " << percentInTop10 << endl
						<< "Manhattan distance between true author and most likely predicted author: " << avgManhattanDistance << endl
						<< "Variance of manhattan distances: " << variance << endl << endl;

					cout << "Cumulative top 10 most likely authors: [" << idMostLikely << ", " << idSecond << ", " << idThird << ", " << idFourth << ", " << idFifth
						<< ", " << idSixth << ", " << idSeventh << ", " << idEighth << ", " << idNinth << ", " << idTenth << "]" << endl << endl;
				}

	/**************************************************************************************************/
	// STEP 4: Get total and cumulative results and distribution

	double percentCumulativeCorrectPredictions = (double)cumulativeNumCorrect / (double)81 * (double)100;
    double percentCumulativeInTop5 = (double)cumulativeTopFive / (double)81 * (double)100;
	double percentCumulativeInTop10 = (double)cumulativeTopTen / (double)81 * (double)100;

	cout << "FINAL results for all 81 authors:" << endl << endl;

	cout << "% of times the true author appeared in the cumulative top 1: " << percentCumulativeCorrectPredictions << endl
		<< "% ... in the cumulative top 5: " << percentCumulativeInTop5 << endl
		<< "% ... in the cumulative top 10: " << percentCumulativeInTop10 << endl << endl;
	
	cout << "% of the TOTAL (81 x 50) top 10 predictions in which each author appeared:" << endl;
	for (int i = 0; i < 81; i++)
		cout << "Author " << i << ": " << (predictionDistribution[i] / ((double)50 * (double)81)) * (double)100 << "%" << endl;
	cout << endl << endl;

	cout << "% of the CUMULATIVE (81) top 10 predictions in which each author appeared:" << endl;
	for (int i = 0; i < 81; i++)
		cout << "Author " << i << ": " << (cumulativePredictionDistribution[i] / (double)81) * (double)100 << "%" << endl;
	cout << endl;

	/**************************************************************************************************/

	return 0;
}

void readFileWordsIntoVector(vector<string> &vec, string filename)
{
	ifstream inFile;
	inFile.open(filename);

	if (!inFile) // Failure case
	{
		cout << "Error opening vocabulary file." << endl;
		return;
	}

	string word;
	set<string> seenWords;

	while(inFile >> word) // Read a word
	{
		// Convert the word to all lowercase & remove punctuation
		string lowerWord;
		for (char c : word)
		{
			if (!isalpha(c))
				continue;

			lowerWord += (char)tolower(c);
		}

		if (!lowerWord.empty())
		{
			// If the word has already been read, continue to the next word
			if (seenWords.find(lowerWord) != seenWords.end())
				continue;
			
			// If the word has not been seen yet, add it to the vector
			// and the set of seen words
			vec.push_back(lowerWord);
			seenWords.insert(lowerWord);
		}
	}

	inFile.close();
}

void vocabWordLengthDistributionCount(map<int, int>& count, const vector<string>& vocab) 
{
	for (string vWord : vocab)
	{
		int length = vWord.size(); // Get the number of letters in the vocab word

		if (count.find(length) != count.end())
			count[length]++; // If this length has been "seen", update the count
		else
			count[length] = 1; // If this is a new length, add it with a count of 1
	}
}

void vocabWordCharDistributionCount(map<char, int>& count, const vector<string>& vocab)
{
	for (int i = 0; i < vocab.size(); i++)
	{
		for (auto c : vocab[i]) // For every character in the vocab list
		{
			if (count.find(c) != count.end())
				count[c]++; // If this character has been "seen", update the count
			else
				count[c] = 1; // If this is a new character, add it with a count of 1
		}
	}
}

double prCharGivenCharOfState(char charGenerated, char charOfTheState)
{
	// CharGenerated = What we actually touched (typed)
	// CharOfTheState = What we want to type in our mind (our cognitive state)

	// The probability that the person will type the intended key is equal to pr_hit
	if (charGenerated == charOfTheState)
		return testPHit;

	// Create an empty array with 25 elements and populate it 
	// so that the elements increment from 1 to 13, then decrement back to 1
	int missdist[25];
	for (int i = 0; i < 25; i++) {
		if (i < 13)
			missdist[i] = (i + 1);
		else
			missdist[i] = (25 - i);
	}

	// Populate an array so that each element is based on the distance, missdist[i], 
	// between a given key and the intended key
	double exponentialDegrade[25];
	for (int i = 0; i < 25; i++)
		exponentialDegrade[i] = pow((1 / testDegKb), missdist[i]);

	// Sum the elements of exponentialDegrade
	double sum = 0.0;
	for (double degrade : exponentialDegrade)
		sum += degrade;

	// Define the constant used to calculate the desired probability 
	// by dividing the probability of missing the intended key 
	// by the sum obtained in the previous block 
	double constant_x = testPMiss / sum;

	// Get the absolute numerical distance between the given key and intended key
	int distASCII_x_y;
	if ((charGenerated - charOfTheState) > 0)
		distASCII_x_y = charGenerated - charOfTheState;
	else
		distASCII_x_y = (charGenerated - charOfTheState) * (-1);

	// Convert the numerical distance to the theoretical distance if the keys were placed 
	// on the perimeter of a one-dimensional, circular keyboard in alphabetical order
	int distKB_x_y;
	if (distASCII_x_y < (26 - distASCII_x_y))
		distKB_x_y = distASCII_x_y;
	else
		distKB_x_y = (26 - distASCII_x_y);

	// Return the probability, calculated by multiplying (constant) by 
	// (0.5 to the power of the given key's distance from the intended key)
	return constant_x * pow((1 / testDegKb), distKB_x_y);
}

void getPrTableForPossibleInitialStates(double prTable[], int sizeOfTable)
{
	// Populate an array so that each element is based on the distance, i, 
	// between a given state and the initial state
	vector<double> exponentialDegrade;
	for (int i = 1; i <= sizeOfTable; i++)
		exponentialDegrade.push_back(pow(1 / testDegSp, i));

	// Sum the elements of the exponential degrade
	double sum = 0.0;
	for (double degrade : exponentialDegrade)
		sum += degrade;

	// Define the constant used to calculate the desired probability
	// and populate prTable, where each element is the product of
	// the constant and an element of exponentialDegrade
	double scalingConstant = 1 / sum;
	for (int i = 0; i < sizeOfTable; i++)
		prTable[i] = scalingConstant * exponentialDegrade[i];
}

void getPrTableForPossibleNextStates(double prTable[], int sizeOfTable, int currentState)
{
	// Get the distances between the current state and any other state
	vector<int> distances;
	for (int i = 0; i < sizeOfTable; i++)
		distances.push_back(i - currentState);

	//In this scenario, it is impossible for a person to transition back to a previous character state.
	// For any distances between the current state and any other state that are less than or equal to
	// 0 (meaning the 'other' state comes before the current state), the ratio of probability is set
	// to 0.
	vector<double> exponentialDegrade;
	for (double distance : distances) {
		if (distance > 0)
			exponentialDegrade.push_back(pow((1 / testDegSp), distance));
		else
			exponentialDegrade.push_back(0);
	}

	//Sum the ratios calculated above.
	double sum = 0.0;
	for (double degrade : exponentialDegrade)
		sum += degrade;

	double scalingConstant = testPMoveOn / sum;

	// Populate prTable based on the constant and exponentialDegrade
	for (int i = 0; i < sizeOfTable; i++) 
		prTable[i] = scalingConstant * exponentialDegrade[i];

	// The probability of transitioning from the current state to itself is
	// equal to the global pRepeat
	prTable[currentState] = testPRepeat;
}

int take1SampleFrom1PrSpace(double prTable[], int sizeOfTable)
{
	int i;
	double prSum = 0;

	for (i = 0; i < sizeOfTable; i++)
		prSum += prTable[i];
	if (prSum < 0.999 || prSum > 1.001)
		cout << "Something is wrong with a random sampleing" << endl
		<< "The sum of all probabilities in the table is " << prSum << endl;

	//Calculate the probability intervals of the characters
	double prAccumulated = 0;
	double* prIntervals;
	prIntervals = new double[sizeOfTable];
	for (i = 0; i < sizeOfTable; i++)
	{
		prAccumulated = prAccumulated + prTable[i];
		prIntervals[i] = prAccumulated;
	}

	// Generate a random number in [0,1]
	i = rand() % 1001;
	double temp = i / 1001.0;
	//cout << "The random number pair generated is " << i << endl << temp << endl;

	bool sampleTaken = false;
	for (i = 0; i < sizeOfTable && !sampleTaken; i++)
		if (temp <= prIntervals[i])
		{
			delete[] prIntervals;
			//cout << "The random interval id is " << i << endl;
			return i;
		}
	return sizeOfTable - 1;
}

void getKeyboardProbabilityTable(char charToType, double prTable[])
{
	string keys = "abcdefghijklmnopqrstuvwxyz";
	for (int i = 0; i < 26; i++) {
		prTable[i] = prCharGivenCharOfState(keys[i], charToType);
	}
}

char typeOneChar(char charToType)
{
	string keys = "abcdefghijklmnopqrstuvwxyz";

	double prTable[26];
	getKeyboardProbabilityTable(charToType, prTable);

	//Sample a character according to probabilities stored in prTable, and
	//	and then return the character.
	int indexOfCharRandomlyGenerated = take1SampleFrom1PrSpace(prTable, 26);

	return keys[indexOfCharRandomlyGenerated];
}

string typeOneWord(string word)
{
	string output;
	int wordSize = word.length();

	// Probability tables for possible initial and next states
	double* prTable = new double[wordSize];
	double* transitionPrTable = new double[wordSize + 1];

	// For storing the current state, the desired character, and the character actually typed
	int currentState;
	char charToType;
	char charTyped;

	// Determine a random initial state after leaving I and its corresponding character in 'word'
	getPrTableForPossibleInitialStates(prTable, wordSize);
	currentState = take1SampleFrom1PrSpace(prTable, wordSize);
	charToType = word[currentState];

	// Determine the character actually typed and store it in 'output'
	charTyped = typeOneChar(charToType);
	output += charTyped;

	// Until the state transitions into the final state, get the next state...
	while (currentState < wordSize) {
		getPrTableForPossibleNextStates(transitionPrTable, (wordSize + 1), currentState);
		currentState = take1SampleFrom1PrSpace(transitionPrTable, (wordSize + 1));

		// ...If the next state is not the final state, determine the character typed and store it in the next element of 'output'
		if (currentState < wordSize) {
			charToType = word[currentState];
			charTyped = typeOneChar(charToType);
			output += charTyped;
		}
	}

	delete[] prTable;
	delete[] transitionPrTable;

	return output;
}

double prOf1CharSeriesWhenTypingOneWord(string observed, string original)
{
	int i, j, k;
	int wordSize = original.length();
	string keys = "abcdefghijklmnopqrstuvwxyz";

	// Get the initial state probabilities
	double* vector_pi = new double[wordSize];
	getPrTableForPossibleInitialStates(vector_pi, wordSize);

	// Get matrices A and B
	double** matrix_A = new double*[wordSize];
	double** matrix_B = new double* [wordSize];

	for (int i = 0; i < wordSize; i++)
	{
		matrix_A[i] = new double[wordSize + 1];
		getPrTableForPossibleNextStates(matrix_A[i], wordSize+1, i);

		matrix_B[i] = new double[26];
		for (int j = 0; j < 26; j++)
			matrix_B[i][j] = prCharGivenCharOfState(keys[j], original[i]);
	}

	// Get transitionProbabilities, observationProbabilities, and columnProbabilities
	int observationIndex = 0;
	char charObserved = observed[observationIndex];
	int indexOfObservedCharInAlphabet = charObserved - 'a';

	double* transitionProbabilities = new double[wordSize];
	double* observationProbabilities = new double[wordSize];
	double* columnProbabilities = new double[wordSize];


	for (i = 0; i < wordSize; i++) 
	{
		transitionProbabilities[i] = vector_pi[i];
		observationProbabilities[i] = matrix_B[i][indexOfObservedCharInAlphabet];
	}

	for (i = 0; i < wordSize; i++)
		columnProbabilities[i] = transitionProbabilities[i] * observationProbabilities[i];

	// Get transitionProbabilities, observationProbabilities, and columnProbabilities (cont.)
	for (i = 1; i < observed.length(); i++) 
	{
		charObserved = observed[i];
		indexOfObservedCharInAlphabet = charObserved - 'a';

		for (j = 0; j < wordSize; j++) 
		{
			transitionProbabilities[j] = 0.0;

			for (k = 0; k < (wordSize + 1); k++)
				transitionProbabilities[j] += (columnProbabilities[j] * matrix_A[j][k]);

			observationProbabilities[j] = matrix_B[j][indexOfObservedCharInAlphabet];
		}

		for (j = 0; j < wordSize; j++)
			columnProbabilities[j] = transitionProbabilities[j] * observationProbabilities[j];

	}

	// Get the probability
	double pr = 0;
	for (i = 0; i < wordSize; i++)
		pr += (columnProbabilities[i] * matrix_A[i][wordSize]);


	// Get rid of the heap memory
	delete[] vector_pi;

	for (int i = 0; i < wordSize; i++)
	{
		delete[] matrix_A[i];
		delete[] matrix_B[i];
	}
	delete[] matrix_A;
	delete[] matrix_B;

	delete[] transitionProbabilities;
	delete[] observationProbabilities;
	delete[] columnProbabilities;

	return pr; // Return the probability
} 

void getProbabilitySumGivenCorruptedWord(vector<pair<double, int>>& output, vector<string> vocab, string wordGenerated, int id)
{
	// For a given possible author...

	double sumPr = 0.0;
	for (string vWord : vocab)
	{
		// Add the probability of seeing typedWord given the current vocabWord to sumPr.
		sumPr += prOf1CharSeriesWhenTypingOneWord(wordGenerated, vWord);
	}

	// Add the natural log of the probability sum and the author id to the output vector.
	// The author who produced the greatest probability log is assumed to be the most
	//  likely author.
	output.push_back(make_pair(log(sumPr), id)); 
}

void sort(vector<pair<double, int>>& vec)
{
	// Order the vector from greatest to least using bubble sort
	for (int i = 0; i < vec.size() - 1; i++)
		for (int j = i + 1; j < vec.size(); j++)
			if (vec[i].first < vec[j].first)
			{
				// Swap if the left element is less than the right element
				double tempPr = vec[i].first;
				int tempId = vec[i].second;

				vec[i].first = vec[j].first;
				vec[i].second = vec[j].second;

				vec[j].first = tempPr;
				vec[j].second = tempId;
			}
}