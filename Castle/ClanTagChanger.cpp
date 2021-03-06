#include "Features.h"
SendClanTagFn SendClanTag;
char Settings::ClanTagChanger::value[30] = "";
bool Settings::ClanTagChanger::animation = false;
int Settings::ClanTagChanger::animationSpeed = 650;
bool Settings::ClanTagChanger::enabled = false; // TODO find a way to go back to the "official" clan tag for the player? -- Save the current clan tag, before editing, then restore it later
ClanTagType Settings::ClanTagChanger::type = ClanTagType::STATIC;

ClanTagChanger::Animation ClanTagChanger::Marquee(std::string name, std::wstring text, int width /*= 15*/)
{
	text.erase(std::remove(text.begin(), text.end(), '\0'), text.end());

	std::wstring cropString = std::wstring(width, ' ') + text + std::wstring(width - 1, ' ');

	std::vector<ClanTagChanger::Frame> frames;
	for (unsigned long i = 0; i < text.length() + width; i++)
		frames.push_back(ClanTagChanger::Frame(cropString.substr(i, width + i), Settings::ClanTagChanger::animationSpeed));

	return ClanTagChanger::Animation(name, frames, ClanTagChanger::ANIM_LOOP);
}

std::vector<std::wstring> splitWords(std::wstring text)
{
	std::wistringstream stream(text);
	std::wstring word;
	std::vector<std::wstring> words;
	while (stream >> word)
		words.push_back(word);

	return words;
}

ClanTagChanger::Animation ClanTagChanger::Words(std::string name, std::wstring text)
{
	// Outputs a word by word animation
	std::vector<std::wstring> words = splitWords(text);
	std::vector<ClanTagChanger::Frame> frames;
	for (unsigned long i = 0; i < words.size(); i++)
		frames.push_back(Frame(words[i], Settings::ClanTagChanger::animationSpeed));

	return ClanTagChanger::Animation(name, frames, ClanTagChanger::ANIM_LOOP);
}

ClanTagChanger::Animation ClanTagChanger::Letters(std::string name, std::wstring text)
{
	// Outputs a letter incrementing animation
	std::vector<ClanTagChanger::Frame> frames;
	for (unsigned long i = 1; i <= text.length(); i++)
		frames.push_back(Frame(text.substr(0, i), Settings::ClanTagChanger::animationSpeed));

	for (unsigned long i = text.length() - 2; i > 0; i--)
		frames.push_back(Frame(frames[i].text, Settings::ClanTagChanger::animationSpeed));

	return ClanTagChanger::Animation(name, frames, ClanTagChanger::ANIM_LOOP);
}

std::vector<ClanTagChanger::Animation> ClanTagChanger::animations = {
	ClanTagChanger::Words("Resurrection", L"Resurrection"),
	ClanTagChanger::Letters("Resurrection", L"Resurrection")
};
ClanTagChanger::Animation* ClanTagChanger::animation = &ClanTagChanger::animations[0];

void ClanTagChanger::UpdateClanTagCallback()
{
	if (strlen(Settings::ClanTagChanger::value) > 0 && Settings::ClanTagChanger::type > ClanTagType::STATIC)
	{
		std::wstring wc = Util::StringToWstring(Settings::ClanTagChanger::value);

		switch (Settings::ClanTagChanger::type)
		{
		case ClanTagType::MARQUEE:
			*ClanTagChanger::animation = ClanTagChanger::Marquee("CUSTOM", wc);
			break;
		case ClanTagType::WORDS:
			*ClanTagChanger::animation = ClanTagChanger::Words("CUSTOM", wc);
			break;
		case ClanTagType::LETTERS:
			*ClanTagChanger::animation = ClanTagChanger::Letters("CUSTOM", wc);
			break;
		default:
			break;
		}

		return;
	}

	ClanTagChanger::animations = {
		ClanTagChanger::Words("Resurrection", L"Resurrection"),
		ClanTagChanger::Letters("Resurrection", L"Resurrection")
	};

	int current_animation = (int)Settings::ClanTagChanger::type - 1;
	if (current_animation >= 0)
		ClanTagChanger::animation = &ClanTagChanger::animations[current_animation];
}

void ClanTagChanger::BeginFrame(float frameTime)
{
	static auto pSetClanTag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>(FindPattern("engine.dll", "\x53\x56\x57\x8B\xDA\x8B\xF9\xFF\x15", "xxxxxxxxx"));
	if (!Settings::ClanTagChanger::enabled)
		return;

	if (!pEngine->IsInGame())
		return;

	long currentTime_ms = Util::GetEpochTime();
	static long timeStamp = currentTime_ms;

	if (currentTime_ms - timeStamp > ClanTagChanger::animation->GetCurrentFrame().time)
	{
		timeStamp = currentTime_ms;
		ClanTagChanger::animation->NextFrame();
	}

	std::string ctWithEscapesProcessed = std::string(Settings::ClanTagChanger::value);
	Util::StdReplaceStr(ctWithEscapesProcessed, "\\n", "\n"); // compute time impact? also, referential so i assume RAII builtin cleans it up...

	if (Settings::ClanTagChanger::type == ClanTagType::STATIC)
		pSetClanTag(ctWithEscapesProcessed.c_str(), "");
	else
		pSetClanTag(Util::WstringToString(ClanTagChanger::animation->GetCurrentFrame().text).c_str(), "");
}
