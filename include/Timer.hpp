//	Diese Datei ist Teil der Tesseract-Engine
//	Phoenix (c) 2017
//	 ___________________           ____________
//	|_______    ________|         |    ________|
//			|  |				  |   |
//			|  |				  |   |
//			|  |	 _________    |   |_____
//			|  |	/	____  \   |    _____|
//			|  |	|  |____|  |  |   | 
//			|  |	|   _______|  |   |
//			|  |	|  |_______   |   |________ 
//			|__|	\_________/   |____________|
//			

#pragma once

//Eine Klasse zum Messen der Zeit
class Timer
{
	public:
		Timer();
	
		float TotalTime()const;  // in Sekunden
		float DeltaTime()const; // in Sekunden
	
		void Reset(); // Aufrufen um den Timer zu resetten
		void Start(); // Aufrufen um den Timer wieder zu starten
		void Stop();  // Aufrufen um den Timer zu stoppen
		void Tick();  // Jedes Frame aufrufen
	
	private:
		double mSecondsPerCount;
		double mDeltaTime;
	
		__int64 mBaseTime;
		__int64 mPausedTime;
		__int64 mStopTime;
		__int64 mPrevTime;
		__int64 mCurrTime;
	
		bool mStopped;
};