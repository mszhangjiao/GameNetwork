#pragma once

// I wanted to implement a simple card game, but ran out of time...
typedef uint8_t Card;
typedef uint8_t Score;

typedef deque<Card> CardList;
typedef vector<CardList> DealtCards;
typedef map<PlayerId, CardList> PlayerDealtCards;

typedef map<PlayerId, Card> PlayerPlayedCards;

// to simplify it, we just set fixed number of players for this game,
// can be extended to min, max player numbers;
class CardDeck
{
public:
	static DealtCards DealCards(int maxCardNum, int playerNumLimit)
	{
		vector<Card> m_Cards;

		m_Cards.resize(maxCardNum);

		for (int i = 0; i < maxCardNum; ++i)
		{
			m_Cards[i] = i + 1;
		}

		int cardNumPerPlayer = maxCardNum / playerNumLimit;

		DealtCards dealtCards;
		dealtCards.resize(playerNumLimit);

		std::random_shuffle(m_Cards.begin(), m_Cards.end());

		auto it = m_Cards.cbegin();
		for (int i = 0; i < playerNumLimit; ++i)
		{
			dealtCards[i].insert(dealtCards[i].cend(), it, it + cardNumPerPlayer);
			// shuffle the cards again;
			std::random_shuffle(dealtCards[i].begin(), dealtCards[i].end());
			it += cardNumPerPlayer;
		}

		return dealtCards;
	}
};
