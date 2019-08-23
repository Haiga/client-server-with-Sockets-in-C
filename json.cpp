#include <iostream>
#include <string>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

const int ERRO = 1;
const int SUCESSO = 0;
const int TAMANHO_BUFFER = 4096;


int main(int argc, char *argv) {
	string strEntrada(TAMANHO_BUFFER, 0), strSaida(TAMANHO_BUFFER, 0), strTemp(TAMANHO_BUFFER, 0);
	while(1) {
		cout << "--> ";
		cin >> strEntrada;

		json j1 = json::array({ {"comando", "enviaMensagem"}, {"valor", strEntrada} });
		strTemp = j1.dump();
		cout << strTemp << endl;

		json j2 = json::parse(strTemp);
		strSaida = j2.dump();
		cout << strSaida << endl;

		cout << j2.get<string>() << endl;
		cout << j2.get<string>() << endl;

		strEntrada.clear();
		strSaida.clear();
		strTemp.clear();
	}

	return SUCESSO;
}