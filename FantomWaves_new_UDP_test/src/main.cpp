#include <iostream>
#include <FantomWaves.h>

struct State
{
	enum StateEnum
	{
		prepare,
		sending,
		waiting,
		prepare_p2p,
		sending_p2p,
		waiting_p2p,
		finish
	};
};


class God
{
public:
	void input();
	void prepare(int & state);
	void sending(int & state);
	void waiting(int & state);
	void prepare_p2p(int & state);
	void sending_p2p(int & state);
	void waiting_p2p(int & state);



private:
	fw::NetSurfer server_info;
	fw::UDP_cliant cliant;
	fw::NetSurfer another_info;
	time_t began_time;
	fw::P2P peer;
};


void God::input()
{
	int state = State::prepare;

	std::cout << "�T�[�o�[��IP�A�h���X����͂��Ă��������B" << std::endl;
	int a, b, c, d;
	scanf("%d.%d.%d.%d", &a, &b, &c, &d);
	fw::IP ip(a, b, c, d);

	std::cout << "�T�[�o�[�̃|�[�g�ԍ�����͂��Ă��������B" << std::endl;
	unsigned short port;
	std::cin >> port;

	std::cout << "�}�b�`���O���ł��B���΂炭���҂����������c" << std::endl;

	server_info.set(ip, port);
}

void God::prepare(int & state)
{
	if (cliant.init(server_info))
	{
		state = State::sending;
		return;
	}

	std::cout << "�l�b�g���[�N�ʐM�������ɃG���[���������܂����B" << std::endl;
	std::cout << "������x�ʐM�����݂܂����H[yes, no]" << std::endl;
	std::string text;
	std::cin >> text;
	if (text != "yes")
	{
		state = State::finish;
	}
}

void God::sending(int & state)
{
	fw::Bindata data;
	data.add(std::string("original matching"));
	data.add(std::string("I wanna match the another"));
	if (cliant.send(data))
	{
		state = State::waiting;
		began_time = time(NULL);
		return;
	}

	std::cout << "�l�b�g���[�N�ʐM���ɃG���[���������܂����B" << std::endl;
	std::cout << "������x�ʐM�����݂܂����H[yes, no]" << std::endl;
	std::string text;
	std::cin >> text;
	if (text != "yes")
	{
		state=State::finish;
	}
}

void God::waiting(int & state)
{
	if (cliant.did_receive())
	{
		fw::Bindata data;
		cliant.pop_received_data(data);
		std::string signature;
		data >> signature;
		if (signature != "original matching")
		{
			return;
		}

		std::string command;
		data >> command;

		if (command == "plz wait another")
		{
			std::cout << "�ΐ푊�肪�����̂�҂��Ă��܂��B���΂炭���҂����������c" << std::endl;
		}

		if (command != "you matched with...")
		{
			return;
		}

		data >> another_info;
		state = State::prepare_p2p;
		std::cout << "�ΐ푊�肪������܂����B�ڑ����J�n���܂��c" << std::endl;
	}
	else
	{
		auto now_time = time(NULL);
		if (now_time - began_time >= 6)
		{
			std::cout << "�T�[�o�[����̉������Ȃ��悤�ł��B" << std::endl;
			std::cout << "������x�ʐM�����݂܂����H[yes, no]" << std::endl;
			std::string text;
			std::cin >> text;
			if (text == "yes")
			{
				state = State::sending;
			}
			else
			{
				began_time = now_time;
			}
		}
	}
}

void God::prepare_p2p(int & state)
{
	if (peer.start())
	{
		state = State::sending_p2p;
		return;
	}

	std::cout << "�ΐ푊��Ƃ̒ʐM�������ɃG���[���������܂����B" << std::endl;
	std::cout << "������x�ʐM�����݂܂����H[yes, no]" << std::endl;
	std::string text;
	std::cin >> text;
	if (text != "yes")
	{
		state = State::finish;
	}
}

void God::sending_p2p(int & state)
{
	fw::Bindata data;
	data.add(std::string("hello"));
	if (peer.send(another_info, data))
	{
		state = State::waiting_p2p;
		return;
	}

	std::cout << "�ΐ푊��ւ̃��b�Z�[�W���M���ɃG���[���������܂����B" << std::endl;
	std::cout << "������x�ʐM�����݂܂����H[yes, no]" << std::endl;
	std::string text;
	std::cin >> text;
	if (text != "yes")
	{
		state = State::finish;
	}
}

void God::waiting_p2p(int & state)
{
	if (peer.are_there_any_left_datas())
	{
		fw::Bindata data;
		fw::NetSurfer surfer;
		peer.pop_received_data(data, surfer);
		std::string message;
		data >> message;
		std::cout << "you got a message:" << std::endl;
		std::cout << message << std::endl;
	}
}




int main()
{
	God god;
	god.input();

	int state = State::prepare;

	while (true)
	{
		if (state == State::prepare){ god.prepare(state); }
		if (state == State::sending){ god.sending(state); }
		if (state == State::waiting){ god.waiting(state); }
		if (state == State::prepare_p2p){ god.prepare_p2p(state); }
		if (state == State::sending_p2p){ god.sending_p2p(state); }
		if (state == State::waiting_p2p){ god.waiting_p2p(state); }
		if (state == State::finish){ break; }
	}

	return 0;
}