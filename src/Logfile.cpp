#include "..\include\Logfile.hpp"

Logfile::Logfile() {}


Logfile::~Logfile()
{
	fprintf(file, "<hr></body></html>");
	fflush(file);
	fclose(file);
}

void Logfile::createLogfile() {
	createLogfile("-unknown-");
}
void Logfile::createLogfile(const char * version)
{
	fopen_s(&file, "log.htm", "w");

	fprintf(file, "<html><head><title>%s</title></head><body style=\"color: #ffffff; background-color: #202020; font-family: Lucida Sans;\"><h1>%s</h1>",
		getTime("log_%F").c_str(), "Logfile");

	std::string config1;
	std::string config2;
#ifdef _DEBUG
	config1 = "DEBUG";
#else
	config1 = "RELEASE";
#endif
	if (sizeof(void*) == 8)
		config2 = "x64";
	else
		config2 = "x86";

	fprintf(file, "<p style=\"color:#B070ff;\">%s%s</p><p style=\"color:#B070ff;\">%s%s %s</p><p style=\"color:#B070ff;\">%s%s</p><hr>", 
		"Datum: ", getTime("%D %X").c_str(), 
		"Konfiguration: ", config1.c_str(), config2.c_str(), 
		"Version: ", version);
	fflush(file);

	fclose(file);
	fopen_s(&file, "log.htm", "a");
}

void Logfile::print(const char * text)
{
	fprintf(file, "<p>%s%s</p>", getTime("[%X] ").c_str(), text);
	fflush(file);
}

void Logfile::print(int color, const char * text)
{
	fprintf(file, "<p style=\"color:#%s\";>%s%s</p>", colors[color].c_str(), getTime("[%X] ").c_str(), text);
	fflush(file);
}

void Logfile::printf(const char * text, ...)
{
	char bufferm[1024];
	va_list args;

	va_start(args, text);
	vsprintf_s(bufferm, (sizeof(bufferm)/sizeof(*bufferm)), text, args);
	va_end(args);

	fprintf(file, "<p>%s%s</p>", getTime("[%X] ").c_str(), bufferm);
	fflush(file);
}

void Logfile::printf(int color, const char * text, ...)
{
	//TCHAR buffer[1024];
	char bufferm[1024];
	va_list args;

	va_start(args, text);
	//vsprintf_s(buffer, text, args);
	vsprintf_s(bufferm, (sizeof(bufferm) / sizeof(*bufferm)), text, args);
	va_end(args);

	fprintf(file, "<p style=\"color:#%s\";>%s%s</p>", colors[color].c_str(), getTime("[%X] ").c_str(), bufferm);
	fflush(file);
}

void Logfile::error(bool critical, const char* msg, const char* filename, const char* function, int line) {
	get()->printf(critical ? TE_ERROR : TE_WARNING, "%s in %s (function %s, line %i): %s",
		critical ? "Error" : "Warning", filename, function, line, msg);

	WCHAR buffer[256];
	swprintf(buffer, 256, L"%s in %hs\n(function %hs, line %i)\n%hs\nOpen Logfile?",
		critical ? L"Error" : L"Warning", filename, function, line, msg);
	int userAction = MessageBoxW(NULL, buffer, critical ? L"Error" : L"Warning",critical ? MB_YESNO | MB_ICONERROR : MB_YESNO | MB_ICONWARNING);
	if (userAction == IDYES) {
		fclose(file);
		ShellExecute(NULL, L"open", L"log.htm", NULL, NULL, SW_SHOWNORMAL);
		Sleep(1000);
		fopen_s(&file, "log.htm", "a");

	}
}

std::string Logfile::getTime(char * format)
{
	time_t now = time(0);
	struct tm time_info;
	localtime_s(&time_info, &now);
	char buffer[60];

	strftime(buffer, sizeof(buffer), format, &time_info);
	return buffer;
}

std::string Logfile::colors[] = { "00FFFF", "FF6000", "808080", "20FF00", "0080FF", "FF0000", "D0D000" };