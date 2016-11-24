#pragma once

// I wanted to implement a simple card game, but ran out of time...
typedef uint8_t Card;
typedef uint8_t Score;

typedef vector<Card> CardList;
typedef vector<vector<Card>> DealtCards;
typedef map<PlayerId, CardList> PlayerDealtCards;

typedef map<PlayerId, Card> PlayerPlayedCards;

// to simplify it, we just set fixed number of players for this game,
// can be extended to min, max player numbers;
class CardDeck
{
public:
	static const uint8_t cMaxCardNum = 10;
	static const uint8_t cPlayerNumLimit = 2;

	static DealtCards DealCards()
	{
		vector<Card> m_Cards;

		m_Cards.resize(cMaxCardNum);

		for (int i = 0; i < cMaxCardNum; ++i)
		{
			m_Cards[i] = i + 1;
		}

		int cardNumPerPlayer = cMaxCardNum / cPlayerNumLimit;

		DealtCards dealtCards;
		dealtCards.resize(cPlayerNumLimit);

		std::random_shuffle(m_Cards.begin(), m_Cards.end());

		auto it = m_Cards.cbegin();
		for (int i = 0; i < cPlayerNumLimit; ++i)
		{
			dealtCards[i].insert(dealtCards[i].cend(), it, it + cardNumPerPlayer);
			it += cardNumPerPlayer;
		}

		return dealtCards;
	}
};
