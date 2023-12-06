#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
using namespace std;

//Одна карта
class Card {
public:
	//Значимость карты
	enum rank { ACE = 1, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING };
	
	//Масть карты
	enum suit { CLUBS, DIAMONDS, HEARTS, SPADES };

	friend ostream& operator<<(ostream& os, const Card& aCard);
	Card(rank r = ACE, suit s = SPADES, bool ifu = true);

	//Получить значение карты
	int GetValue() const;
	
	//Перевернуть карту (скрыть/показать)
	void Flip();
private:
	rank m_Rank;
	suit m_Suit;
	bool m_IsFaceUp;
};
Card::Card(rank r, suit s, bool ifu) :
	m_Rank(r), m_Suit(s), m_IsFaceUp(ifu)
{}
int Card::GetValue() const {
	int value = 0;
	if (m_IsFaceUp) {
		value = m_Rank;
		if (value > 10)
			value = 10;
	}
	return value;
}
void Card::Flip() {
	m_IsFaceUp = !m_IsFaceUp;
}

//Рука игрока
class Hand {
public:
	Hand();
	virtual ~Hand();
	
	//добавить карту в руку
	void Add(Card* pCard);
	
	//очистить руку
	void Clear();
	
	//Узнать общую сумму карт в руке
	int GetTotal() const;
protected:
	vector<Card*> m_Cards;
};
Hand::Hand() {
	m_Cards.reserve(7);
}
Hand::~Hand() {
	Clear();
}
void Hand::Add(Card* pCard) {
	m_Cards.push_back(pCard);
}
void Hand::Clear() {
	for (vector<Card*>::iterator iter = m_Cards.begin(); iter != m_Cards.end(); iter++) {
		delete* iter;
		*iter = 0;
	}
	m_Cards.clear();
}
int Hand::GetTotal() const {
	if (m_Cards.empty()) {
		return 0;
	}
	if (m_Cards[0]->GetValue() == 0) {
		return 0;
	}
	int total = 0;
	for (vector<Card*>::const_iterator iter = m_Cards.begin(); iter != m_Cards.end(); ++iter) {
		total += (*iter)->GetValue();
	}
	bool ContainsAce = false;
	for (vector<Card*>::const_iterator iter = m_Cards.begin(); iter != m_Cards.end(); ++iter) {
		if ((*iter)->GetValue() == Card::ACE) {
			ContainsAce = true;
		}
	}
	if (ContainsAce && total <= 11) {
		total += 10;
	}
	return total;
}

//Абстрактный класс игрока
class GenericPlayer : public Hand {
	friend ostream& operator<<(ostream& os, const GenericPlayer& aGP);
public:
	GenericPlayer(const string& name = "");
	virtual ~GenericPlayer();
	virtual bool IsHitting() const = 0;
	
	//вылетел ли игрок?
	bool IsBusted() const;
	
	//функция вылета
	void Bust() const;
protected:
	string m_Name;
};
GenericPlayer::GenericPlayer(const string& name) :
	m_Name(name)
{}
GenericPlayer::~GenericPlayer() {}
bool GenericPlayer::IsBusted() const {
	return (GetTotal() > 21);
}
void GenericPlayer::Bust() const {
	cout << m_Name << " bust." << endl;
}

//Игрок
class Player : public GenericPlayer {
public:
	Player(const string& name = "");
	virtual ~Player();
	
	//Хочет ли игрок взять карту?
	virtual bool IsHitting() const;
	
	//Игрок победил
	void Win() const;
	
	//Игрок проиграл
	void Lose() const;
	
	//Игрок сыграл ничью
	void Push() const;
};
Player::Player(const string& name) :
	GenericPlayer(name)
{}
Player::~Player() {}
bool Player::IsHitting() const {
	cout << m_Name << ". do you want a hit? (y/n): ";
	char response;
	cin >> response;
	return (response == 'y' || response == 'Y');
}
void Player::Win() const {
	cout << m_Name << " wins!" << endl;
}
void Player::Lose() const {
	cout << m_Name << " loses." << endl;
}
void Player::Push() const {
	cout << m_Name << " pushes." << endl;
}

//Дилер
class House : public GenericPlayer {
public:
	House(const string& name = "Dealer");
	virtual ~House();
	
	//Собирается ли диллер брать еще карты
	virtual bool IsHitting() const;
	
	//перевернуть первую карту
	void FlipFirstCard();
};
House::House(const string& name) :
	GenericPlayer(name)
{}
House::~House()
{}
bool House::IsHitting() const {
	return (GetTotal() <= 16);
}
void House::FlipFirstCard() {
	if (!(m_Cards.empty())) {
		m_Cards[0]->Flip();
	}
	else {
		cout << "No cards to flip!" << endl;
	}
}

//Стол
class Deck : public Hand {
public:
	Deck();
	virtual ~Deck();
	//Заполнить картами
	void Populate();

	//Перемешать карты
	void Shuffle();
	
	//Раздать по 2 карты каждому
	void Deal(Hand& aHand);
	
	//Добавить 1 карту
	void AdditionalCards(GenericPlayer& aGenericPlayer);
};
Deck::Deck() {
	m_Cards.reserve(52);
	Populate();
}
Deck::~Deck() {}
void Deck::Populate() {
	Clear();
	for (int s = Card::CLUBS; s < Card::SPADES; ++s) {
		for (int r = Card::ACE; r < Card::KING; ++r) {
			Add(new Card(static_cast<Card::rank>(r), static_cast<Card::suit>(s)));
		}
	}
}
void Deck::Shuffle() {
	random_shuffle(m_Cards.begin(), m_Cards.end());
}
void Deck::Deal(Hand& aHand) {
	if (!m_Cards.empty()) {
		aHand.Add(m_Cards.back());
		m_Cards.pop_back();
	}
	else {
		cout << "Out of cards. Unable to deal." << endl;
	}
}
void Deck::AdditionalCards(GenericPlayer& aGenericPlayer) {
	cout << endl;
	while (!(aGenericPlayer.IsBusted()) && aGenericPlayer.IsHitting()) {
		Deal(aGenericPlayer);
		cout << aGenericPlayer << endl;
		if (aGenericPlayer.IsBusted()) {
			aGenericPlayer.Bust();
		}
	}
}

//Игровой процесс
class Game {
public:
	Game(const vector<string>& names);
	~Game();

	//Начать игру
	void Play();
private:
	Deck m_Deck;
	House m_House;
	vector<Player> m_Players;
};
Game::Game(const vector<string>& names) {
	for (vector<string>::const_iterator pName = names.begin(); pName != names.end(); ++pName) {
		m_Players.push_back(Player(*pName));
	}
	srand(static_cast<unsigned int>(time(0)));
	m_Deck.Populate();
	m_Deck.Shuffle();
}
Game::~Game() {}
void Game::Play() {
	vector<Player>::iterator pPl;
	for (int i = 0; i < 2; ++i) {
		for (pPl = m_Players.begin(); pPl != m_Players.end(); ++pPl) {
			m_Deck.Deal(*pPl);
		}
		m_Deck.Deal(m_House);
	}
	m_House.FlipFirstCard();
	for (pPl = m_Players.begin(); pPl != m_Players.end(); ++pPl) {
		cout << *pPl << endl;
	}
	cout << m_House << endl;

	for (pPl = m_Players.begin(); pPl != m_Players.end(); ++pPl) {
		m_Deck.AdditionalCards(*pPl);
	}
	m_House.FlipFirstCard();
	cout << endl << m_House;

	m_Deck.AdditionalCards(m_House);
	if (m_House.IsBusted()) {
		for (pPl = m_Players.begin(); pPl != m_Players.end(); ++pPl) {
			if (!(pPl->IsBusted())) {
				pPl->Win();
			}
		}
	}
	else {
		for (pPl = m_Players.begin(); pPl != m_Players.end(); ++pPl) {
			if (!(pPl->IsBusted())) {
				if (pPl->GetTotal() > m_House.GetTotal()) {
					pPl->Win();
				}
				else if (pPl->GetTotal() < m_House.GetTotal()) {
					pPl->Lose();
				}
				else {
					pPl->Push();
				}
			}
		}
	}

	for (pPl = m_Players.begin(); pPl != m_Players.end(); ++pPl) {
		pPl->Clear();
	}
	m_House.Clear();
}


ostream& operator<<(ostream& os, const Card& aCard);
ostream& operator<<(ostream& os, const GenericPlayer& aGenericPlayer);

int main() {
	cout << "=====================" << endl;
	cout << "Welcome to BlackJack!" << endl;
	cout << "=====================" << endl;

	int numplayers = 0;
	while (numplayers < 1 || numplayers>7) {
		cout << "With how many players will be play? (1-7): ";
		cin >> numplayers;
	}
	vector<string> names;
	string name;
	for (int i = 0; i < numplayers; ++i) {
		cout << "\nEnter player name: ";
		cin >> name;
		names.push_back(name);
	}
	cout << endl;

	Game aGame(names);
	char again = 'y';
	while (again != 'n' && again != 'N') {
		cout << endl;
		aGame.Play();
		cout << "Do you want to play again? (y/n): ";
		cin >> again;
	}
	return 0;
}


ostream& operator<<(ostream& os, const Card& aCard) {
	const string RANKS[] = { "0", "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K" };
	const string SUITS[] = { "c", "d", "h", "s" };
	if (aCard.m_IsFaceUp) {
		os << RANKS[aCard.m_Rank] << SUITS[aCard.m_Suit];
	}
	else {
		os << "XX";
	}
	return os;
}

ostream& operator<<(ostream& os, const GenericPlayer& aGenericPlayer) {
	os << aGenericPlayer.m_Name << ":\t";
	vector<Card*>::const_iterator pCard;
	if (!aGenericPlayer.m_Cards.empty()) {
		for (pCard = aGenericPlayer.m_Cards.begin(); pCard != aGenericPlayer.m_Cards.end(); ++pCard) {
			os << *(*pCard) << "\t";
		}
		if (aGenericPlayer.GetTotal() != 0) {
			os << "(" << aGenericPlayer.GetTotal() << ")";
		}
	}
	else {
		os << "<empty>";
	}
	return os;
}