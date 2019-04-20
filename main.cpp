#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <cmath>
#include <math.h>
#include <random>

using namespace std;

//double randomDouble(double lower_bound, double upper_bound)
//{
//	std::uniform_real_distribution<double> unif(lower_bound, upper_bound);
//	std::default_random_engine re;
//	double a_random_double = unif(re);
//
//	return a_random_double;
//}

double randomDouble(double fMin, double fMax)
{
	double f = (double)rand() / RAND_MAX;
	return fMin + f * (fMax - fMin);
}


class Agent
{
public:
	
	vector<int> priorities;

	double capOfMoney;
	double money;



	void randomMoney(int low, int high) {
		//money = rand() % (high - low) + low;
		double fatTailLow = log(low);
		double fatTailHigh = log(high);
		double result = randomDouble(fatTailLow, fatTailHigh);
		money = exp(result);
		capOfMoney = money;
	}

	void generatePriorities(int numIn) {
		int num = numIn - 1;
		for (int i = 0; i < numIn; ++i) {
			bool inserted = false;
			while (inserted == false) {
				int randNum = rand() % numIn;
				bool alreadyIn = false;
				for (int i = 0; i < priorities.size(); ++i) {
					if (priorities[i] == randNum) {
						alreadyIn = true;
					}
				}
				if (alreadyIn == false) {
					priorities.push_back(randNum);
					inserted = true;
				}
			}

		}
	}

};

class Charity {
public:
	vector <int> agentsDonated;
	double fundingCap;
	double bankAccount;

	void randomCap(int low, int high) {
		fundingCap = rand() % (high - low) + low;
	}

};


//void blind(vector<Agent> &agentsIn, vector<Charity> &charitiesIn) {
//	for (int i = 0; i < agentsIn.size(); ++i) {
//		int firstChoice = agentsIn[i].priorities[0];
//		double amount = agentsIn[i].money;
//		charitiesIn[firstChoice].bankAccount += amount;
//	}
//}

bool compare(const pair<int, int>&i, const pair<int, int>&j)
{
	return i.second > j.second;
}

bool compareAgents(const Agent a, const Agent b) {
	return a.capOfMoney > b.capOfMoney;
}

class ABM {

public:
	
	double totalInitialAgentMoney;
	double currentTotalAgentMoney;
	double totalCharityFundingCap;


	vector< Agent > listOfAgents;
	vector< Charity > charities;


	vector< Agent > averagedBlindAgents;
	vector< Charity > averagedBlindCharities;

	vector< Agent > averagedwocAgents;
	vector< Charity > averagedwocCharities;

	vector< Agent > averagedlocalAgents;
	vector< Charity > averagedlocalCharities;

	ofstream *blindfile;
	ofstream *wocfile;
	ofstream *localfile;

	int numAgents;
	int moneyLow;
	int moneyHigh;

	int numCharities;
	int fundingLow;
	int fundingHigh;


	void blind() {
		for (int i = 0; i < listOfAgents.size(); ++i) {
			int firstChoice = listOfAgents[i].priorities[0];
			double amount = listOfAgents[i].money;
			
			charities[firstChoice].bankAccount += amount;
		}
	}


	//done
	//generates an ordered list of priorities based off of agents, return a sorted vector of pair <charity, total> >
	vector< pair <int, int> > priorities(int agentIDLow, int agentIDHigh) {
		vector< pair<int, int> > sol;
		for (int i = 0; i < charities.size(); ++i) {
			pair<int, int> pHolder = make_pair(i, 0);
			sol.push_back(pHolder);
		}
		//for each priority
		for (int i = 0; i < charities.size(); ++i) {
			int charityCount = 0;
			//for each agent
			for (int a = agentIDLow; a < agentIDHigh; ++a) {
				//check to see if agent's priority 
				if (i == listOfAgents[a].priorities[0]) {
					++charityCount;
				}
			}
			sol[i].second = charityCount;

		}
		sort(sol.begin(), sol.end(), compare);
		return sol;
	}

	void wisdomOfCrowds() {

		double totalAgentMoney = 0;
		//add up all the agent's money
		for (int i = 0; i < listOfAgents.size(); ++i) {
			totalAgentMoney += listOfAgents[i].money;
		}

		double remainingMoney = totalAgentMoney;
		//calls a function
		vector < pair <int, int > > gloPriorities = priorities(0, listOfAgents.size());

		//give money to charities
		for (int i = 0; i < charities.size(); ++i) {
			double currentFundingCap = charities[gloPriorities[i].first].fundingCap;

			// if agents collectivly have more money than the current funding cap
			if (remainingMoney > currentFundingCap) {
				for (int a = 0; a < listOfAgents.size(); ++a) {
					double percentage = listOfAgents[a].money / totalAgentMoney;
					double amountDonated = percentage * currentFundingCap;
					listOfAgents[a].money -= amountDonated;
					charities[gloPriorities[i].first].bankAccount += amountDonated;
				}
				remainingMoney -= currentFundingCap;
				totalAgentMoney -= currentFundingCap;
			}
			else {
				for (int a = 0; a < listOfAgents.size(); ++a) {
					double percentage = listOfAgents[a].money / totalAgentMoney;
					double amountDonated = percentage * remainingMoney;
					listOfAgents[a].money -= amountDonated;
					charities[gloPriorities[i].first].bankAccount += amountDonated;
				}
				break;
			}
		}

	}

	void localConverge() {
		sort(listOfAgents.begin(), listOfAgents.end(), compareAgents);
		for (int i = 1; i < listOfAgents.size(); ++i) {
			int chance = round(returnPercentage(listOfAgents[i - 1].money, listOfAgents[i].money));
			double outcome = rand() % 100;
			if (outcome < chance) {
				setAgentBPrioritiesToA(listOfAgents[i - 1], listOfAgents[i]);
			}
		}

		blind();
	}

	void addToAverageBlind() {
		for (int i = 0; i < listOfAgents.size(); ++i) {
			averagedBlindAgents[i].capOfMoney += listOfAgents[i].capOfMoney;
		}

		for (int i = 0; i < charities.size(); ++i) {
			averagedBlindCharities[i].fundingCap += charities[i].fundingCap;
			averagedBlindCharities[i].bankAccount += charities[i].bankAccount;
		}
	}

	void addToAverageWoc() {
		for (int i = 0; i < listOfAgents.size(); ++i) {
			averagedwocAgents[i].capOfMoney += listOfAgents[i].capOfMoney;
			averagedwocAgents[i].money += listOfAgents[i].money;
		}

		for (int i = 0; i < charities.size(); ++i) {
			averagedwocCharities[i].fundingCap += charities[i].fundingCap;
			averagedwocCharities[i].bankAccount += charities[i].bankAccount;
		}
	}

	void addToAverageLocal() {
		for (int i = 0; i < listOfAgents.size(); ++i) {
			averagedlocalAgents[i].capOfMoney += listOfAgents[i].capOfMoney;
			averagedlocalAgents[i].money += listOfAgents[i].money;
		}

		for (int i = 0; i < charities.size(); ++i) {
			averagedlocalCharities[i].fundingCap += charities[i].fundingCap;
			averagedlocalCharities[i].bankAccount += charities[i].bankAccount;
		}
	}

	void runNtimes(int times) {
		
		averagedBlindAgents.resize(numAgents);
		averagedBlindCharities.resize(numCharities);

		averagedwocAgents.resize(numAgents);
		averagedwocCharities.resize(numCharities);

		averagedlocalAgents.resize(numAgents);
		averagedlocalCharities.resize(numCharities);

		for (int i = 0; i < times; ++i) {
			setup();
			blind();
			if (i == 0) {
				printCharityState(blindfile);
				printAgents(blindfile);
			}
			addToAverageBlind();
			
			clearCharities();

			wisdomOfCrowds();

			if (i == 0) {
				printCharityState(wocfile);
				printAgents(wocfile);
			}

			addToAverageWoc();

			clearCharities();
			resetMoney();

			if (i == 0) {
				*localfile << "before priorities" << endl;
				for (int i = 0; i < listOfAgents.size(); ++i) {
					*localfile << i << "  ";
					for (int a = 0; a < charities.size(); ++a) {
						*localfile << listOfAgents[i].priorities[a] << "  ";
					}
					*localfile << endl;

				}
				*localfile << endl;
			}

			localConverge();

			if (i == 0) {
				printCharityState(localfile);
				printAgents(localfile);

				*localfile << "end priorities" << endl;
				for (int i = 0; i < listOfAgents.size(); ++i) {
					*localfile << i << "  ";
					for (int a = 0; a < charities.size(); ++a) {
						*localfile << listOfAgents[i].priorities[a] << "  ";
					}
					*localfile << endl;

				}
				*localfile << endl;
			}
			addToAverageLocal();
		}
		for (int i = 0; i < listOfAgents.size(); ++i) {
			averagedBlindAgents[i].capOfMoney /= times;
			averagedlocalAgents[i].capOfMoney /= times;
			averagedwocAgents[i].capOfMoney /= times;

			averagedwocAgents[i].money /= times;
			averagedlocalAgents[i].money /= times;
		}

		for (int i = 0; i < charities.size(); ++i) {
			averagedBlindCharities[i].bankAccount /= times;
			averagedlocalCharities[i].bankAccount /= times;
			averagedwocCharities[i].bankAccount /= times;

			averagedBlindCharities[i].fundingCap /= times;
			averagedlocalCharities[i].fundingCap /= times;
			averagedwocCharities[i].fundingCap /= times;
		}

		listOfAgents = averagedBlindAgents;
		charities = averagedBlindCharities;
		printCharityState(blindfile);
		printAgents(blindfile);

		listOfAgents = averagedwocAgents;
		charities = averagedwocCharities;
		printCharityState(wocfile);
		printAgents(wocfile);

		listOfAgents = averagedlocalAgents;
		charities = averagedlocalCharities;
		printCharityState(localfile);
		printAgents(localfile);


	}

	void setup() {

		listOfAgents.clear();
		charities.clear();

		for (int i = 0; i < numAgents; ++i) {
			Agent aHolder;
			aHolder.randomMoney(moneyLow, moneyHigh);
			aHolder.generatePriorities(numCharities);
			listOfAgents.push_back(aHolder);
		}
		for (int i = 0; i < numCharities; ++i) {
			Charity cHolder;
			cHolder.randomCap(fundingLow, fundingHigh);
			charities.push_back(cHolder);
		}
	}


	void printCharityState(ofstream *fileIn) {


		//prints charity names at the top of the page

		for (int i = 0; i < charities.size(); ++i) {
			*fileIn << i << "      ";
		}
		*fileIn << '\n';

		for (int i = 0; i < charities.size(); ++i) {
			*fileIn << charities[i].fundingCap << "   ";
		}
		*fileIn << '\n';

		for (int i = 0; i < charities.size(); ++i) {
			*fileIn << charities[i].bankAccount << "   ";
		}
		*fileIn << '\n';
		*fileIn << '\n';

		//fileIn << "Blind Mode" << endl;
		//fileIn << endl;
		//fileIn << "Charities" << endl;
		//fileIn << endl;

		//for (int i = 0; i < charities.size(); ++i) {
		//	fileIn << "Charity: " << i << "   Bank Account: " << charities[i].bankAccount << endl;
		//}

		//fileIn << endl;
		//fileIn << "Agents" << endl;
		//fileIn << endl;

		//for (int i = 0; i < listOfAgents.size(); ++i) {
		//	fileIn << "Agent: " << "    Money Left: " << listOfAgents[i].money << endl;
		//}

	}

	void printAgents(ofstream *fileIn) {
		*fileIn << "Agents" << endl;

		for (int i = 0; i < listOfAgents.size(); ++i) {
			*fileIn << i << "   " << listOfAgents[i].capOfMoney << "   " << listOfAgents[i].money << endl;
		}
		*fileIn << '\n';
		*fileIn << '\n';
	}
	

	double returnPercentage(double a, double b) {
		if (a == b) {
			return 100;
		}
		if (a < b) {
			return (a / b) * 100;
		}
		else {
			return (b / a) * 100;
		}
	}

	void clearCharities() {
		for (int i = 0; i < charities.size(); ++i) {
			charities[i].bankAccount = 0;
			charities[i].agentsDonated.clear();
		}
	}

	void resetMoney() {
		for (int i = 0; i < listOfAgents.size(); ++i) {
			listOfAgents[i].money = listOfAgents[i].capOfMoney;
		}
	}

	void setAgentBPrioritiesToA(Agent &a, Agent &b) {
		for (int i = 0; i < b.priorities.size(); ++i) {
			b.priorities[i] = a.priorities[i];
		}
	}



};



int main(int argc, char * argv[]) {

	ofstream* fileBlind = new ofstream;
	fileBlind->open("blind.txt");

	ofstream* fileWOC = new ofstream;
	fileWOC->open("woc.txt");

	ofstream* fileLocal = new ofstream;
	fileLocal->open("local.txt");

	ABM model;
	model.blindfile = fileBlind;
	model.wocfile = fileWOC;
	model.localfile = fileLocal;

	model.numAgents = 10;
	model.moneyLow = 50;
	model.moneyHigh = 1000;

	model.numCharities = 3;
	model.fundingLow = 800;
	model.fundingHigh = 1200;

	model.runNtimes(1000000);

	//model.setup();

	//model.blind();

	//model.printCharityState(fileBlind);

	//model.clearCharities();

	//model.wisdomOfCrowds();

	//model.printCharityState(fileWOC);


	fileBlind->close();
	fileWOC->close();

}