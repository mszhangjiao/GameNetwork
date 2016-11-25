#pragma once

#include <string>

using namespace std;

// it defines client specific features, again the "game" is supper simple now:
// after connecting to a server, the client simply receives the heart beats from server and show them;
class GameClient : public MiniGame
{
public:
	enum ECommand
	{
		Com_Start = -1,
		Com_Replay,
		Com_Leave,
		Com_End,
	};

	static const size_t cCommandLength = 256;

	static bool StaticInit(const string& serverIP, const string& playerName);

	virtual void ProcessPacket(uint8_t msgType, NetPlayerPtr playerPtr, InputBitStream& is) override;

	void PushCommand(const char* command);
	bool PopCommand(string& command);
	void RunCommand();

	static void ConsoleCommandProc(GameClient *game);
	void ExecConsoleCommand();

	string GetSupportedCommands() const;
private:
	GameClient(const string& serverIP, const string& playerName);
	virtual void DoFrame() override;
	void UpdateLocalPlayer();
	
	void FindMatch();
	void PlayTurn(TurnId turnId);
	void PromptUser(const string& message);
	void LeaveGame();
	void MatchReady();

	void HandleFindMatchReply(InputBitStream& is);
	void HandleJoinMatch(InputBitStream& is);
	void HandleStartMatch(InputBitStream& is);
	void HandleStartTurn(InputBitStream& is);
	void HandleEndTurn(InputBitStream& is);
	void HandleEndMatch(InputBitStream& is);

	bool InitNetManager();

	string CardsToString()
	{
		string s;
		for (auto card : m_CardList)
		{
			s += " ";
			s += to_string(card);
		}

		return s;
	}

	string m_ServerIP;
	string m_PlayerName;
	MatchPtr m_CurrentMatchPtr;
	CardList m_CardList;
	size_t m_InitialCardsNum;

	HANDLE m_Thread;
	CriticalSection m_CSection;
	queue<string> m_Commands;

	static const char* cCommandList[];
};