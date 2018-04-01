#include <iostream>
#include <fstream>
#include <cmath>
#include <map>
#include <list>
#include <ctime>
#include <cstring>
#include <algorithm>
#include <climits>

using namespace std;

typedef struct {
	unsigned int a, b, x, y, s, f, d;
	unsigned int finalTime, score;
	bool notGood;
} TRide;

typedef struct {
	unsigned int x, y, t, carId;
} TCar;

typedef struct {
	unsigned int ride_id, finalTime, score;
} TAssignement;

unsigned int R, C, F, N, B, T, cN;
int globalScore = -1;
map< unsigned int, list< unsigned int > > globalSituation;
clock_t starttime;
float interval;

bool rideSorter(const TRide& first, const TRide& second){
	if(first.d != second.d)
		return (first.d > second.d);
	if(first.s != second.s)
		return (first.s > second.s);
	return (first.f > second.f);
}

bool rideSorter1(const TRide& first, const TRide& second){
	if(first.s != second.s)
		return (first.s < second.s);
	return (first.d < second.d);
}

bool rideSorter2(const TRide& first, const TRide& second){
	if(first.notGood == true)
		return false;
	if(second.notGood == true)
		return true;
	if(first.score != second.score)
		return (first.score > second.score);
	if(first.s != second.s)
		return (first.s < second.s);
	return (first.d < second.d);
}

bool carSorter(const TCar& first, const TCar& second){
	return (first.t < second.t);
}

bool carSorter1(const TCar& first, const TCar& second){
	return (first.t > second.t);
}

clock_t printTime = clock(), afterPrint;

void regression(list<TRide> *rides, TCar *cars, int currentScore,
	map< unsigned int, list< unsigned int > > *currentSituation,
	unsigned int maximumObtainableScore, unsigned int carId,
	unsigned int biggestTime) {

	afterPrint = clock();
	if((( float )(afterPrint - printTime))/CLOCKS_PER_SEC > 0.5){
		cout<< "carId: " << carId << " "
			<< "rideSize=" << rides->size() << " "
			<< "currScore=" << currentScore << " "
			<< "maximumObtainableScore + currScore=" << maximumObtainableScore + currentScore << " "
			<< endl;
		printTime = afterPrint;
	}
	// cout<< "carId: " << carId << "; ridesSize: " << rides->size() << endl;

	if(rides->size() < F - carId)
		regression(rides, cars, currentScore, currentSituation, maximumObtainableScore, carId + 1,
			biggestTime);

	list<TRide>::iterator itRide = rides->begin();
	unsigned int cN;
	for(cN = 0; cN < rides->size(); cN++){
		if( (( float )(clock() - starttime))/CLOCKS_PER_SEC > interval )
			return;

		itRide->notGood = false;

		unsigned int score = 0, waitingTime, distance_from_starting, finalTime = cars[carId].t;

		distance_from_starting = abs(itRide->a - cars[carId].x) + abs(itRide->b - cars[carId].y);
		waitingTime = (cars[carId].t + distance_from_starting < itRide->s)
			? itRide->s - (cars[carId].t + distance_from_starting) : 0;
		finalTime += distance_from_starting + waitingTime + itRide->d;
		if(finalTime >= T){
			itRide->notGood = true;
			itRide++;
			continue;
		}
		if(finalTime < itRide->f)
			score += itRide->d;

		if(cars[carId].t + distance_from_starting <= itRide->s)
			score += B;

		if(globalScore != -1
			&& currentScore + score + maximumObtainableScore - itRide->d - B <= globalScore){
			itRide->notGood = true;
			itRide++;
			continue;
		}

		if(rides->size() == 1){
			if(globalScore == -1 || currentScore + score > globalScore){
				globalScore = currentScore + score;
				globalSituation = map< unsigned int, list< unsigned int > >(*currentSituation);
				cout << "new GS: " <<globalScore << endl;
			}
			itRide->notGood = true;
			itRide++;
			continue;
		}

		itRide->finalTime = finalTime;
		itRide->score = score;

		if(finalTime > biggestTime)
			biggestTime = finalTime;
		itRide++;
	}
	rides->sort(rideSorter2);

	itRide = rides->begin();
	for(cN = 0; cN < rides->size(); cN++){

		if(itRide->notGood)
			break;

		TCar newCars[F];
		memcpy((void*)newCars, (void*)cars, F * sizeof(TCar));

		list<TRide> newRides (*rides);
		list<TRide>::iterator elem = newRides.begin();
		advance(elem, cN);

		newCars[carId].x = elem->x;
		newCars[carId].y = elem->y;
		newCars[carId].t = itRide->finalTime;

		newRides.erase(elem);

		map< unsigned int, list< unsigned int > > newSit (*currentSituation);

		// cout << "\tscore for " << N-cN << " :" << score << endl;

		if(carId == F - 1){
			list<TRide>::iterator itRide1 = newRides.begin();
			unsigned int a = newRides.size();
			while(a--){

				if(itRide1->d > biggestTime)
					return;

				itRide1++;
			}
			sort(newCars, newCars + F, carSorter);
			regression(&newRides, newCars, currentScore + itRide->score, &newSit,
					maximumObtainableScore - itRide->d - B, 0, 0);
		} else {
			regression(&newRides, newCars, currentScore + itRide->score, &newSit,
					maximumObtainableScore - itRide->d - B, carId + 1, biggestTime);
		}

		itRide++;
	}

}

int main(int argc, char* argv[]){

	fstream myfile(argv[1], ios::in);

	myfile >> R >> C >> F >> N >> B >> T;

	unsigned int maximumObtainableCost = 0;
	TRide rides[N], *itRide = rides;
	cN = N;
	while(cN--){

		myfile >> itRide->a >> itRide->b >> itRide->x >>
			itRide->y >> itRide->s >> itRide->f;

		itRide->d = abs(((long)itRide->a) - itRide->x)
		+ abs(((long)itRide->b) - itRide->y);

		// cout << itRide->a << itRide->b << itRide->x <<
		// 	itRide->y << itRide->s << itRide->f << endl;

		maximumObtainableCost += itRide->d + B;

		itRide++;
	}

	// cout << maximumObtainableCost << endl;

	myfile.close();

	TCar cars[F], *itCars = cars;
	cN = 0;
	while(cN < F){

		itCars->x = itCars->y = itCars->t = 0;
		itCars->carId = cN;

		itCars++;
		cN++;
	}

	starttime = clock();
	interval = 1800.0;
	map< unsigned int, list< unsigned int > > newSit;
	list<TRide> rrr (rides, rides + sizeof(rides) / sizeof(TRide));

	rrr.sort(rideSorter1);
	regression(&rrr, cars, 0, &newSit, maximumObtainableCost, 0, 0);

	cout << "global score: " << globalScore << endl;

	return 0;
}