#pragma once

#include <string>

using namespace std;

// it defines client specific features, again the "game" is supper simple now:
// after connecting to a server, the client simply receives the heart beats from server and show them;
class GameClient : public MiniGame
{
public:
	static bool StaticInit(const string& serverIP, const string& playerName);

	virtual void ProcessPacket(uint8_t msgType, NetPlayerPtr playerPtr, InputBitStream& is) override;

private:
	GameClient(const string& serverIP, const string& playerName);
	virtual void DoFrame() override;
	void UpdateLocalPlayer();
	
	void FindMatch();
	void PlayTurn(TurnId turnId);
	void PromptToReplay();
	void LeaveGame();
	void MatchReady();

	void HandleFindMatchReply(InputBitStream& is);
	void HandleJoinMatch(InputBitStream& is);
	void HandleStartMatch(InputBitStream& is);
	void HandleStartTurn(InputBitStream& is);
	void HandleEndTurn(InputBitStream& is);
	void HandleEndMatch(InputBitStream& is);

	bool InitNetManager();

	string m_ServerIP;
	string m_PlayerName;
	MatchPtr m_CurrentMatchPtr;
	CardList m_CardList;
	size_t m_InitialCardsNum;
};