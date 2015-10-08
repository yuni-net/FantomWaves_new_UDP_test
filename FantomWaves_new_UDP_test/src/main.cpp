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

	std::cout << "サーバーのIPアドレスを入力してください。" << std::endl;
	int a, b, c, d;
	scanf("%d.%d.%d.%d", &a, &b, &c, &d);
	fw::IP ip(a, b, c, d);

	std::cout << "サーバーのポート番号を入力してください。" << std::endl;
	unsigned short port;
	std::cin >> port;

	std::cout << "マッチング中です。しばらくお待ちください…" << std::endl;

	server_info.set(ip, port);
}

void God::prepare(int & state)
{
	if (peer.start())
	{
		state = State::sending;
		return;
	}

	std::cout << "ネットワーク通信準備中にエラーが発生しました。" << std::endl;
	std::cout << "もう一度通信を試みますか？[yes, no]" << std::endl;
	std::string text;
	std::cin >> text;
	if (text != "yes")
	{
		state = State::finish;
	}
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

	std::cout << "ネットワーク通信中にエラーが発生しました。" << std::endl;
	std::cout << "もう一度通信を試みますか？[yes, no]" << std::endl;
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

		data.add(std::string());	// 文字列化するため
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
			std::cout << "対戦相手が現れるのを待っています。しばらくお待ちください…" << std::endl;
			began_time = time(NULL);
			continue;
		}

		if (command == "you can match with ...")
		{
			state = State::prepare_p2p;
			std::cout << "対戦相手が見つかりました。接続を開始します…" << std::endl;
			set_another_info(root);
			return;
		}

		std::cout << "invalid command." << std::endl;
	}

	auto now_time = time(NULL);
	if (now_time - began_time >= 6)
	{
		std::cout << "サーバーからの応答がないようです。" << std::endl;
		std::cout << "もう一度通信を試みますか？[yes, no]" << std::endl;
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

void God::prepare_p2p(int & state)
{
	state = State::sending_p2p;
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

	std::cout << "対戦相手へのメッセージ送信中にエラーが発生しました。" << std::endl;
	std::cout << "もう一度通信を試みますか？[yes, no]" << std::endl;
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