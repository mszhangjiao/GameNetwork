#pragma once

// I wanted to implement a simple card game, but ran out of time...
typedef uint8_t Card;
class CardDeck
{
public:
	const uint8_t cMaxCardNum = 52;
	const uint8_t cMaxPlayerNum = 2;

	CardDeck()
	{
		m_Cards.resize(cMaxCardNum);
		m_DealtCards.resize(cMaxPlayerNum);
	}

	void CreateCards()
	{
		for (int i = 0; i < cMaxCardNum; ++i)
			m_Cards[i] = i + 1;
	}

	void ShuffleCards()
	{
		std::random_shuffle(m_Cards.begin(), m_Cards.end());
	}

	void DealCards()
	{
		int cardNumPerPlayer = cMaxCardNum / cMaxPlayerNum;

		auto it = m_Cards.cbegin();
		for (int i = 0; i < cMaxPlayerNum; ++i)
		{
			m_DealtCards[i].insert(m_DealtCards[i].cend(), it, it + cardNumPerPlayer);
			it += cardNumPerPlayer;
		}
	}

	const vector<Card>& GetDealtCardsForPlayer(int i) const
	{
		assert(i >= 0 && i < cMaxPlayerNum);
		return m_DealtCards[i];
	}

private:
	vector<Card> m_Cards;
	vector<vector<Card>> m_DealtCards;
};
