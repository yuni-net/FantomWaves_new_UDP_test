#include <iostream>
#include <FantomWaves.h>
#include "picojson.h"

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
	fw::NetSurfer another_info;
	time_t began_time;
	fw::P2P peer;

	void set_another_info(picojson::object & root);
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
	state = State::sending;
}

void God::sending(int & state)
{
	std::string request =
		fw::cnct() <<
		"{ " <<
		"\"signature\": \"original message\", " <<
		"\"command\": \"I wanna match another\", " <<
		"\"local_ip\": \"" << fw::NetWork::get_my_address_text(0) << "\", " <<
		"\"local_port\": \"" << peer.get_port() << "\"" <<=
		" }";
	fw::Bindata data;
	data.add(request);
	if (peer.send(server_info, data))
	{
		std::cout << "I send the request" << std::endl;
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
	while(peer.are_there_any_left_datas())
	{
		std::cout << "you got a message" << std::endl;

		fw::Bindata data;
		fw::NetSurfer from;
		peer.pop_received_data(data, from);

		data.add(std::string());	// �����񉻂��邽��
		std::cout << "data bytes: " << data.bytes() << std::endl;
		std::cout << "received data: " << std::endl;
		std::cout << data.buffer() << std::endl;

		picojson::value json_data;
		std::string error = picojson::parse(json_data, data.buffer());
		if (error.empty() == false)
		{
			std::cout << "that's not json style." << std::endl;
			continue;
		}

		picojson::object & root = json_data.get<picojson::object>();

		const std::string & signature = root["signature"].get<std::string>();
		if (signature != "original message")
		{
			std::cout << "invalid signature." << std::endl;
			continue;
		}

		const std::string & command = root["command"].get<std::string>();
		if (command == "wait another")
		{
			std::cout << "�ΐ푊�肪�����̂�҂��Ă��܂��B���΂炭���҂����������c" << std::endl;
			began_time = time(NULL);
			continue;
		}

		if (command == "you can match with ...")
		{
			state = State::prepare_p2p;
			std::cout << "�ΐ푊�肪������܂����B�ڑ����J�n���܂��c" << std::endl;
			set_another_info(root);
			return;
		}

		std::cout << "invalid command." << std::endl;
	}

}

void God::prepare_p2p(int & state)
{
	state = State::sending_p2p;
}

void God::sending_p2p(int & state)
{
	std::cout << "���M���������b�Z�[�W����͂��Ă�������" << std::endl;
	std::string message;
	std::cin >> message;

	if (message == "")
	{
		state = State::waiting_p2p;
		return;
	}

	fw::Bindata data;
	data.add(message);
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

	state = State::sending_p2p;
}

void God::set_another_info(picojson::object & root)
{
	std::string ip_text = root["global_ip"].get<std::string>();
	int port = int(root["global_port"].get<double>());
	fw::IP ip;
	sockaddr_in address;
	address.sin_addr.S_un.S_addr = inet_addr(ip_text.c_str());
	ip.set(address);
	another_info.set(ip, port);
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