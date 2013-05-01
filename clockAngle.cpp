/*
e-mail: sasha-kurlov@yandex.ru
*/
#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>
#include <time.h>
#define _USE_MATH_DEFINES
#include <math.h>

#define ARGS_AMOUNT 4
#define LEN_OF_THE_12_INPUT_FORMAT 8
#define LEN_OF_THE_24_INPUT_FORMAT 5
#define POST_MERIDIEM "PM"
#define ANTE_MERIDIEM "AM"
#define DEGREES     "deg"
#define RADIANS     "rad"
#define DEG_MIN_SEC "dms"
#define PENDULUM    "mech"
#define QUARTZ      "quar"
#define DEG_BY_HOUR 30		/* 360 / 12 etc */
#define DEG_BY_MIN	 6
#define MINUTES_IN_AN_HOUR 60
#define SECONDS_IN_AN_MINUTE 60
#define SECONDS_IN_AN_HOUR 3600
#define HALF_OF_THE_CIRCLE 180
#define ARGC_TIME_INDEX		1
#define ARGC_FORMAT_INDEX	2
#define ARGC_TYPE_INDEX		3

using namespace std;

class ClockParser
{
    public:
        ClockParser();
		~ClockParser();
		
        bool isValidFormat(string input);
        bool isValidType(string input);
        bool isValidTime(string input);
		bool isValidInput(string inpTime, string inpFormat, string inpType);

		struct tm* givenTime;

    private:
        vector<string>* validFormats;
        vector<string>* validClocks;
};

ClockParser::ClockParser()
{
    validFormats = new vector<string>;
    validFormats->push_back(DEGREES);
    validFormats->push_back(RADIANS);
    validFormats->push_back(DEG_MIN_SEC);

    validClocks = new vector<string>;
    validClocks->push_back(PENDULUM);
    validClocks->push_back(QUARTZ);

	givenTime = new struct tm;

}

ClockParser::~ClockParser()
{
	delete validFormats;
	delete validClocks;
	delete givenTime;
}

bool ClockParser::isValidFormat(string input)
{
    return find(validFormats->begin(), validFormats->end(), input) != validFormats->end();
}

bool ClockParser::isValidType(string input)
{
    return find(validClocks->begin(), validClocks->end(), input) != validClocks->end();
}

bool ClockParser::isValidTime(string input)
{
	if (input.size() == LEN_OF_THE_12_INPUT_FORMAT)
	{
		string format = input.substr(6, 2);
		if ((format == POST_MERIDIEM || format == ANTE_MERIDIEM) && (input.substr(2,1) == ":")) 
		{
			givenTime->tm_hour = stoi(input.substr(0,2));
			givenTime->tm_min = stoi(input.substr(3,2));
			if (givenTime->tm_hour > 0 && givenTime->tm_hour < 12 && givenTime->tm_min > 0 && givenTime->tm_min < MINUTES_IN_AN_HOUR)
				return true;
			else return false;
		}
		else return false;
	}
	else if (input.size() == LEN_OF_THE_24_INPUT_FORMAT)
	{
		if (input.substr(2,1) == ":")
		{
			givenTime->tm_hour = stoi(input.substr(0,2));
			givenTime->tm_min = stoi(input.substr(3,2));
			if (givenTime->tm_hour > 0 && givenTime->tm_hour < 24 && givenTime->tm_min > 0 && givenTime->tm_min < MINUTES_IN_AN_HOUR)
				return true;
			else return false;
		}
		else return false;
	}
	else return false;
}

bool ClockParser::isValidInput(string inpTime, string inpFormat, string inpType)
{
	return isValidTime(inpTime) && isValidFormat(inpFormat) && isValidType(inpType);
}

class Clock
{
	public:
		Clock(struct tm* clTime, string clType, string anFormat);
		~Clock();
		string calcAngle();

	private:
		float calcDegAngle(); /* результат принадлежит [0, 180) */ 
		string to_dms(float angle);
		string angleFormat;
		int minutesDeg;
		float hoursDeg;
};

Clock::Clock(struct tm * clTime, string anFormat, string clType)
{
    angleFormat = anFormat;
	minutesDeg = clTime->tm_min * DEG_BY_MIN;
	if (clTime->tm_hour > 11) clTime->tm_hour -= 12; //переводим 24-форматные часы в 12
	if (clType == PENDULUM)  // если часы механические, указываем точное положение часовой стрелки
	{
		hoursDeg = (clTime->tm_hour + clTime->tm_min * 1.0f / MINUTES_IN_AN_HOUR) * DEG_BY_HOUR;
	}
	else					// иначе часовая стрелка может быть только целым числом от 0 до 11
		hoursDeg = clTime->tm_hour * DEG_BY_HOUR;
}

Clock::~Clock()
{

}

string Clock::calcAngle()
{
	string result;
	if (angleFormat == DEGREES)
		result =  to_string(static_cast<long long>(calcDegAngle()));
	if (angleFormat == RADIANS)
		result = to_string(static_cast<long double>(calcDegAngle() * M_PI / HALF_OF_THE_CIRCLE));
	if (angleFormat == DEG_MIN_SEC)
		result = to_dms(calcDegAngle());

	return result;
}

float Clock::calcDegAngle()
{
	float result;
	if (minutesDeg > hoursDeg)
		result = minutesDeg - hoursDeg;
	else 
		result = hoursDeg - minutesDeg;
	if ( hoursDeg - minutesDeg > 180 || minutesDeg - hoursDeg > 180)
		result = HALF_OF_THE_CIRCLE * 2 - result; //инвертируем смежные  углы
	return result;
}

string Clock::to_dms(float angle)
{
	int grad, seconds, min_d, min_e, sec_d, sec_e;
	grad = int(angle);
	seconds = int(SECONDS_IN_AN_HOUR * (angle - grad));
	min_d = seconds / SECONDS_IN_AN_MINUTE / 10; // кол-во десятков минут
	seconds -= min_d * SECONDS_IN_AN_MINUTE * 10;
	min_e = seconds / SECONDS_IN_AN_MINUTE;		//кол-во едениц минут
	seconds -= min_e * SECONDS_IN_AN_MINUTE;
	sec_d = seconds / SECONDS_IN_AN_MINUTE;
	sec_e = seconds % SECONDS_IN_AN_MINUTE;
	std::stringstream sstm;
	sstm << grad << "." << min_d << min_e << "'" << sec_d << sec_e << "''";
	return sstm.str();
}

int main(int argc, char *argv[])
{
    if (argc == ARGS_AMOUNT)
	{
		try
		{
			ClockParser* parser = new ClockParser();
			if (parser == 0)
				throw "Memory allocation failure";
			if ( parser->isValidInput( argv[ARGC_TIME_INDEX], argv[ARGC_FORMAT_INDEX], argv[ARGC_TYPE_INDEX] ) )
			{
				Clock* myClock = new Clock(parser->givenTime, argv[ARGC_FORMAT_INDEX], argv[ARGC_TYPE_INDEX]);
				if (myClock == 0)
					throw "Memory allocation failure";
				cout << myClock->calcAngle() << endl;
				delete parser;
				delete myClock;
			}
			else
				cout << "Invalid input data. Here is an example of the correct: \n03:00 deg mech" << endl;
		}
		catch(char * str)
		{
			cout << "Exception raised: " << str << endl;
		}
		catch(invalid_argument)
		{
			cout << "Exception raised: no conversion could be performed." << endl;
		}
		catch(out_of_range)
		{
			cout << "Exception raised: converted value would fall out of the range of the result type." << endl;
		}
	}
	else
	{
		cout << "incorrect number of arguments. \nPlease, enter <time> <deg/dms/rad> <mech/quar>" << endl;
	}
}
