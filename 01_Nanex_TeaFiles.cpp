#include <stdio.h>
#include <string.h>
#include <process.h>       // Need for threading
#include <windows.h>       // Need for Sleep() function
#include <time.h>
#include "nxcoreapi_class.h"
#include "Common.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <iterator>


typedef std::vector<std::string> stringvec;

// Couple forwards - functions are below
unsigned __stdcall StartProcessTapeThread(void* pVoid);
void OnNxCoreStatus(const NxCoreSystem* pNxCoreSys, const NxCoreMessage* pNxCoreMsg);
int __stdcall OnNxCoreCallback(const NxCoreSystem* pNxCoreSys, const NxCoreMessage* pNxCoreMsg);
void OnNxCoreTrade(const NxCoreSystem* pNxCoreSys, const NxCoreMessage* pNxCoreMsg);
void OnNxCoreExgQuote(const NxCoreSystem* pNxCoreSys, const NxCoreMessage* pNxCoreMsg);
void OnNxCoreMMQuote(const NxCoreSystem* pNxCoreSys, const NxCoreMessage* pNxCoreMsg);
Tick GetRandomTick();
bool isForbidden(char c);
void read_directory(const std::string& name, stringvec& v);
struct path_leaf_string;

// Global Instance of NxCore Class
NxCoreClass NxCore;

unsigned char InThread = 0;

#undef max

teatime::Time clockTime;
using namespace boost::filesystem;
using namespace std;
namespace fs = boost::filesystem;

double BAChangeArray[3];
int TimeArray[4];

// Main Entry Point for app.	
int main(int argc, char* argv[])
{
	char Filename[255];
	clock_t begin = clock();

	printf("Start.\n");

	// If we can load the NxCore DLL (base dll load on processor type)

#ifdef _M_IX86
	if (NxCore.LoadNxCore("...\\C++\\NxCoreAPI.dll"))
#else
	if (NxCore.LoadNxCore("...\\C++\\NxCoreAPI64.dll"))
#endif
	{

		// If a tape filename was passed in command line argument,
		// assign it to FileName	  
		if (argv[1])
			strcpy(Filename, argv[1]);
		else
			strcpy(Filename,"...\\20180316.GS.nx2");

		// Set in Thread  flag
		InThread = 1;

		// Start a new thread!
		unsigned int ThreadAddr;
		_beginthreadex(NULL, 0, StartProcessTapeThread, Filename, 0 , &ThreadAddr);

		// Sleep until the thread exits 
		while (InThread)
			Sleep(100);

		// Unload NxCore DLL
		NxCore.UnloadNxCore();
	}

	printf(" Stop.\n");

	clock_t end = clock();
	double time_spend = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("%lf\n", time_spend);

	// Exit app
	return 0;
}

// Thread function called to start Process Tape
//----------------------------------------------
unsigned __stdcall StartProcessTapeThread(void* pVoid)

{
	// Use control flags to eliminate OPRA quotes.	    
	NxCore.ProcessTape((char*)pVoid,
		NULL,
		NxCF_EXCLUDE_OPRA, 0,
		OnNxCoreCallback);

	InThread = 0;
	return 1;
}


// The NXCore Callback Function	
//-----------------------------
int __stdcall OnNxCoreCallback(const NxCoreSystem* pNxCoreSys, const NxCoreMessage* pNxCoreMsg)
{
	// Do something based on the message type
	switch (pNxCoreMsg->MessageType)
	{

	case NxMSG_EXGQUOTE:
		OnNxCoreExgQuote(pNxCoreSys, pNxCoreMsg);	
		break;

	}

	// Continue running the tape
	return NxCALLBACKRETURN_CONTINUE;
}


// OnNxCoreQuote: Function to handle NxCore ExgQuote messages.	
//--------------------------------------------------------------
void OnNxCoreExgQuote(const NxCoreSystem* pNxCoreSys, const NxCoreMessage* pNxCoreMsg)
{
	//if (strcmp(pNxCoreMsg->coreHeader.pnxStringSymbol->String, "eAAPL") == 0 || strcmp(pNxCoreMsg->coreHeader.pnxStringSymbol->String, "eORCL") == 0) {
		// Assign a pointer to the ExgQuote data
		NxCoreExgQuote* Quote = (NxCoreExgQuote*)&pNxCoreMsg->coreData.ExgQuote;

		// Get bid and ask price
		double Bid = NxCore.PriceToDouble(Quote->coreQuote.BidPrice, Quote->coreQuote.PriceType);
		double Ask = NxCore.PriceToDouble(Quote->coreQuote.AskPrice, Quote->coreQuote.PriceType);
		double AskChange = NxCore.PriceToDouble(Quote->coreQuote.AskPriceChange, Quote->coreQuote.PriceType);
		double BidChange = NxCore.PriceToDouble(Quote->coreQuote.BidPriceChange, Quote->coreQuote.PriceType);
		double BidQuote = NxCore.PriceToDouble(Quote->coreQuote.BidSize, Quote->coreQuote.PriceType);
		double AskQuote = NxCore.PriceToDouble(Quote->coreQuote.AskSize, Quote->coreQuote.PriceType);

		std::string Symbol1 = pNxCoreMsg->coreHeader.pnxStringSymbol->String;
		std::replace_if(Symbol1.begin(), Symbol1.end(), isForbidden, '_');
		std::string Symbol2 = ".tea";
		std::string Symbol3 = Symbol1 + Symbol2;
		char* c = const_cast<char*>(Symbol3.c_str());

		current_path ("C:\\Users\\Manu\\Documents\\03_Working_Files\\02_Data\\01_LOB");
		//path p("C:\\Users\\Manu\\Documents\\03_Working_Files\\02_Data\\01_LOB");
		
			if (BidChange != 0) {

				printf("B,%s,%d-%d-%dT%02d:%02d:%02d.%02d,%0.2f,%d,%0.2f,%d\n",
					pNxCoreMsg->coreHeader.pnxStringSymbol->String,
					pNxCoreMsg->coreHeader.nxSessionDate.Year,
					pNxCoreMsg->coreHeader.nxSessionDate.Month,
					pNxCoreMsg->coreHeader.nxSessionDate.Day,
					pNxCoreMsg->coreHeader.nxExgTimestamp.Hour,
					pNxCoreMsg->coreHeader.nxExgTimestamp.Minute,
					pNxCoreMsg->coreHeader.nxExgTimestamp.Second,
					pNxCoreMsg->coreHeader.nxExgTimestamp.MsOfDay,
					Bid,
					Quote->coreQuote.BidSize,
					BidChange,
					pNxCoreMsg->coreHeader.ReportingExg);
				//Quote->coreQuote.QuoteCondition)



				BAChangeArray[0] = Bid;
				BAChangeArray[1] = BidChange;
				BAChangeArray[2] = BidQuote;

				TimeArray[0] = pNxCoreMsg->coreHeader.nxSessionDate.Year;
				TimeArray[1] = pNxCoreMsg->coreHeader.nxSessionDate.Month;
				TimeArray[2] = pNxCoreMsg->coreHeader.nxSessionDate.Day;
				TimeArray[3] = pNxCoreMsg->coreHeader.nxExgTimestamp.MsOfDay;

				fs::directory_iterator b("....\02_Data\\01_LOB"), e;
				std::vector<fs::path> paths(b, e);

				auto it = find(paths.begin(), paths.end(), Symbol3);
				
				if (paths.empty())
				{
					auto tf = TeaFile<Tick>::Create(c, "Quotes");
					tf->Write(GetRandomTick());
				}
				else if (it != paths.end())
				{
					
					auto tf2 = TeaFile<Tick>::OpenReadWrite(c);
					tf2->Write(GetRandomTick());
				}
				else
				{
					auto tf = TeaFile<Tick>::Create(c, "Quotes");
					tf->Write(GetRandomTick());
				}
				std::fill_n(BAChangeArray, 3, 0);
				std::fill_n(TimeArray, 4, 0);
			}	

		else {


			printf("A,%s,%d-%d-%dT%02d:%02d:%02d.%02d,%0.2f,%d,%0.2f,%d\n",
				pNxCoreMsg->coreHeader.pnxStringSymbol->String,
				pNxCoreMsg->coreHeader.nxSessionDate.Year,
				pNxCoreMsg->coreHeader.nxSessionDate.Month,
				pNxCoreMsg->coreHeader.nxSessionDate.Day,
				pNxCoreMsg->coreHeader.nxExgTimestamp.Hour,
				pNxCoreMsg->coreHeader.nxExgTimestamp.Minute,
				pNxCoreMsg->coreHeader.nxExgTimestamp.Second,
				pNxCoreMsg->coreHeader.nxExgTimestamp.MsOfDay,
				Ask,
				Quote->coreQuote.AskSize,
				AskChange,
				pNxCoreMsg->coreHeader.ReportingExg);

			BAChangeArray[0] = Ask;
			BAChangeArray[1] = AskChange;
			BAChangeArray[2] = AskQuote;

			TimeArray[0] = pNxCoreMsg->coreHeader.nxSessionDate.Year;
			TimeArray[1] = pNxCoreMsg->coreHeader.nxSessionDate.Month;
			TimeArray[2] = pNxCoreMsg->coreHeader.nxSessionDate.Day;
			TimeArray[3] = pNxCoreMsg->coreHeader.nxExgTimestamp.MsOfDay;
		}

}
Tick GetRandomTick()

{
	ptime pt(date(TimeArray[0], TimeArray[1], TimeArray[2]), milliseconds(TimeArray[3]));
	Tick t;
	t.Time = pt;
	t.Bid = BAChangeArray[0];
	t.BidSize = BAChangeArray[2];
	t.BidChange= BAChangeArray[1];
	return t;
}

bool isForbidden(char c)
{
	static std::string forbiddenChars("\\/:?\"<>|");
	return std::string::npos != forbiddenChars.find(c);
}

struct path_leaf_string
{
	std::string operator()(const boost::filesystem::directory_entry& entry) const
	{
		return entry.path().leaf().string();
	}
};

void read_directory(const std::string& name, stringvec& v)
{
	boost::filesystem::path p("...\\02_Data\\01_LOB");
	boost::filesystem::directory_iterator start(p);
	boost::filesystem::directory_iterator end;
	std::transform(start, end, std::back_inserter(v), path_leaf_string());
}

