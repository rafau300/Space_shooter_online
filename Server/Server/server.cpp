//#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS	//¿eby nie wywala³o "This function or variable may be unsafe"

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>	//'socklen_t'
#include <stdio.h>
#include <cstdlib>
#include <time.h>
#include <math.h>

#pragma comment(lib, "ws2_32.lib")	//linkowanie
#pragma comment(lib, "wsock32.lib")	//bibliotek WinSocka

	struct a{ //asteroida
	int x,y,hp,punkty;
	int rx,ry;
	BITMAP *bmp;
	int lvl;
	int wybuch;
	};

	struct statek_kosmiczny {
	int x,y,amunicja;
	int rodzaj_broni;
	int punkty;
	int pocisk_x[20];
	int pocisk_y[20];
	int pocisk_strzelono[20];
	bool zniszczono;
	};

	a asteroida[50];
	statek_kosmiczny statek[4];

	WORD wVersionRequested;
    WSADATA wsaData;
    int kod_bledu;
	char nazwa_hosta[30];
	int dlugosc_nazwy=30;
	struct hostent* host;
	struct in_addr addr;
	char **pAlias;
	int i=0;

	int port;
	char wiadomosc[15] ={0};
	int dlugosc_wiadomosci;
	char adres[32];

	int ilosc_SI = 0;
	int ilosc_asteroid=10;
	int liczba_nieodebranych_pakietow[4];
	bool polaczenie_z_graczem[4];
	int ile_asteroid_pozostalo=ilosc_asteroid;
	bool przyspieszenie=false;
	bool przyspieszenie2=false;
	bool zestrzelono_wszystkie_asteroidy=false;

	int petla=0;
	int brak_polaczenia[4]={0,0,0,0};
	int odebrano;

	struct SztucznaInteligencja {
		int ruch_x;
		int ruch_y;
		int poprzedni_ruch_x;
		int poprzedni_ruch_y;
	};
	SztucznaInteligencja SI[3];

	//protopypy funkcji
	void funkcjaSI ();
	int WSAStartup ();
	int TCP ();
	int UDP ();
	int UDP_asteroidy();
	int main (int argc, wchar_t **argv);
	void strzal(int petla);
	bool wszystkie_asteroidy_zestrzelone=false;


//===================Inicjalizacja WinSocka==============================
	int WSAStartup () {//
	 wVersionRequested = MAKEWORD(2, 2);

    kod_bledu = WSAStartup(wVersionRequested, &wsaData);
    if (kod_bledu != 0) {
        printf("WSAStartup zwrocilo blad nr: %d\n", kod_bledu);
        return 1;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        printf("Nie znaleziono Winsock.dll\n");
        WSACleanup();
        return 1;
    }
    else printf("Winsock 2.2 dziala poprawnie\n");

	kod_bledu=gethostname(nazwa_hosta,dlugosc_nazwy);
	
	if(kod_bledu!=0) {
		printf("Wystapil blad podczas pobierania nazwy hosta nr: %d\n",kod_bledu);
		return 1;
	}
	else printf("Nazwa hosta: %s\n",nazwa_hosta);
	nazwa_hosta[29]='\0';
	return 0;
}

//===================wyœwietlenie komunikatu===========
int DisplayResourceNAMessageBox(int ile_graczy) {

    int msgboxID;

	if (ile_graczy==1) msgboxID = MessageBox(
			NULL,
			(LPCWSTR)L"Gracz nie zglasza sie\nSerwer zostanie wylaczony",
			(LPCWSTR)L"SERWER",
			MB_ICONERROR | MB_OK | MB_DEFBUTTON2
		);

	else if (ile_graczy==3) msgboxID = MessageBox(
			NULL,
			(LPCWSTR)L"Wszyscy gracze nie zglaszaja sie\nSerwer zostanie wylaczony",
			(LPCWSTR)L"SERWER",
			MB_ICONERROR | MB_OK | MB_DEFBUTTON2
		);

    return msgboxID;
}

//============================Kolizja statku=================
int kolizja_statku (void){
	int i,kolizja=0,kolizja_ze_znajdzka=0,kod=0;
	for (int j=1;j<=2;j++) {
	for (i=0;i<ilosc_asteroid;i++) {
	//Tutaj kolizja jest sprawdzana na podstawie kolizji prostokatow
		if ((statek[j].x-10<asteroida[i].x+64 && statek[j].x-10>asteroida[i].x) && (statek[j].y-24<asteroida[i].y+64 && statek[j].y-24>asteroida[i].y)) {
			kolizja=1;  //statek z prawej u dolu
			break; }
		if ((statek[j].x+24<asteroida[i].x+64 && statek[j].x+24>asteroida[i].x) && (statek[j].y+24<asteroida[i].y+64 && statek[j].y+24>asteroida[i].y)) {
			kolizja=1;  //statek z lewej u gory
			break; }
		if ((statek[j].x+10<asteroida[i].x+64 && statek[j].x+10>asteroida[i].x) && (statek[j].y-24<asteroida[i].y+64 && statek[j].y-24>asteroida[i].y)) {
			kolizja=1;  //statek z leewj u dolu
			break; }
		if ((statek[j].x-24<asteroida[i].x+64 && statek[j].x-24>asteroida[i].x) && (statek[j].y+24<asteroida[i].y+64 && statek[j].y+24>asteroida[i].y)) {
			kolizja=1;  //statek z prawej u gory
			break; }
	}
	
	if (kolizja==1) {					//gdy statek zderzy sie z asteroida
		statek[j].x=750;
		statek[j].zniszczono=true;
		kod+=j;
	}
	}
	return kod;
}
//===========================kolizja asteroid===================
void kolizja_asteroid (int a1, int a2) {
	double x1,x2,y1,y2;
	int pom;
	x1=double(asteroida[a1].x);
	x2=double(asteroida[a2].x);
	y1=double(asteroida[a1].y);
	y2=double(asteroida[a2].y);
	//sprawdzenie, czy nastapila kolizja na podstawie rownania okregu
	if(sqrt(double(abs( ( (x1-x2)*(x1-x2) ) )) + double(abs( ( (y1-y2)*(y1-y2) ) ))) <55) {
		if(asteroida[a1].x>asteroida[a2].x) {	//asteroida 1 z prawej
			asteroida[a1].x++;
			asteroida[a2].x--; 
			pom=asteroida[a1].rx;
			asteroida[a1].rx=asteroida[a2].rx;
		    asteroida[a2].rx=pom;}
		else {									//asteroida 1 z lewej
			asteroida[a1].x--;
			asteroida[a2].x++;
			pom=asteroida[a1].rx;
			asteroida[a1].rx=asteroida[a2].rx;
		    asteroida[a2].rx=pom;}
		if(asteroida[a1].y>asteroida[a2].y) {	//asteroida 1 u gory
			asteroida[a1].y++;
			asteroida[a2].y--; 
			pom=asteroida[a1].ry;
			asteroida[a1].ry=asteroida[a2].ry;
		    asteroida[a2].ry=pom;
		}
		else {									//asteroida 1 u dolu
			asteroida[a1].y--;
			asteroida[a2].y++;
			pom=asteroida[a1].ry;
			asteroida[a1].ry=asteroida[a2].ry;
		    asteroida[a2].ry=pom;
		}
	}
}
//===========================ruch asteroid======================
void ruch_asteroid() {//¿eby SI "wiedzia³y", gdzie znajduj¹ siê asteroidy, bez przesylania pakietów ze wspó³rzêdnymi
		for (i=0;i<50;i++) {
		for (int j=0;j<50;j++) {
		if (i==j) continue;
		else
			kolizja_asteroid(i,j);		//kolizja pomiedzy asteroidami
		}}

	if (ilosc_SI == 1) {//w przypadku, gdy gramy z SI, to ruch asteroid jest wykonywany w ka¿dej pêtli
		for(i=0;i<ilosc_asteroid;i++) { 
		if (asteroida[i].hp<=0) continue;
		if(asteroida[i].x<=0||asteroida[i].x>=534) asteroida[i].rx=asteroida[i].rx*-1; //kolizja asteroidy ze sciana
		if(asteroida[i].y>600) asteroida[i].y=-64;		//teleportacja asteroidy
		asteroida[i].x=asteroida[i].x+asteroida[i].rx;	//przesuwanie asteroidy
		asteroida[i].y=asteroida[i].y+asteroida[i].ry;
		if (asteroida[i].hp <=0 && asteroida[i].hp >=-50) {	//gdy asteroida zostala zestrzelona, a nie odjeto jej od ilosc asteroid
			ile_asteroid_pozostalo--;
			asteroida[i].hp=-100;
		}
		if (ile_asteroid_pozostalo >1 && ile_asteroid_pozostalo <= 4 && !przyspieszenie) {
			przyspieszenie=true;
			for (int j=0;j<ilosc_asteroid;j++) asteroida[j].ry++;
		}
		else if (ile_asteroid_pozostalo <= 1 && !przyspieszenie2) {
			przyspieszenie2=true;
			for (int j=0;j<ilosc_asteroid;j++) asteroida[j].ry++;
		}
	}	
	}
}

//=======================Strzelanie======================================
void strzal (int petla) {
		int j=0;

		if (ilosc_SI >0) {
		for (i=1;i<=2;i++) {
			j=0;
			if (statek[i].rodzaj_broni==1) {  //jesli bron to zielony laser
			if (petla%25==0) {				//co 25 petle wykonuj...
				while (statek[i].pocisk_strzelono[j]==1) { //i gdy jest wystrzelony to..
					j++; 
				}
			if (j<20 && statek[i].pocisk_strzelono[j]==false) {
				statek[i].pocisk_x[j]=statek[i].x;
				statek[i].pocisk_y[j]=statek[i].y-20;
				statek[i].pocisk_strzelono[j]=true;
		}}
		}
		}
		}

		for (i=1;i<=2;i++) {
			for (int j=0;j<20;j++) {
				statek[i].pocisk_y[j]-=4;
				if (statek[i].pocisk_y[j]<0) {
					statek[i].pocisk_strzelono[j]=false;
				}
			}
		}


		/*if (statek[nr_gracza].rodzaj_broni==2) {  //czerwony laser(?)
		if (petla%10==0) {
		while (pocisk[i].strzelono==1) {
			i++; }
		if (pocisk[i].strzelono==0) {
			pocisk[i].x=statek[nr_gracza].x;
			pocisk[i].y=statek[nr_gracza].y-20;
			pocisk[i].strzelono=1; 
			if (dzwiek_wlaczony==true) play_sample( laser2, 255, 127, 1000, 0 );
		}}}*/
		/*
		if (key[KEY_SPACE]) {	//odpalenie bomby za pomoca spacji
			if (bomba.ilosc>0 && bomba.uzyto==false) {
				bomba.ilosc-=1;
				bomba.uzyto=true;
				if (dzwiek_wlaczony==true) play_sample(wybuch_sample,130,127,350,0);
				for (i=0;i<ilosc_asteroid;i++) {
					if (asteroida[i].wybuch>=100) continue;
					if (asteroida[i].y>-64  && asteroida[i].x>-64) {
					asteroida[i].hp=0;
					asteroida[i].wybuch=100;
					punkty[nr_gracza]=punkty[nr_gracza]+asteroida[i].punkty;
					asteroida[i].x=-1000;
					ile_pozostalo--;
					}
			}
			}}
	*/
	for (int k=1;k<=2;k++) {
	for (i=0;i<ilosc_asteroid;i++) { //sprawdzenie, czy pocisk uderzyl asteroide
	if (asteroida[i].wybuch>20 || asteroida[i].hp<0) continue;//dodatkowe sprawdzenie czy asteroida wybuchla, ¿eby pomin¹æ zestrzelone
	 int x1=asteroida[i].x, y1=asteroida[i].y, s1=64, w1=64;
	 int x2,y2,s2,w2;
	 for (int j=0;j<20;j++) {
		 if (statek[k].rodzaj_broni==1) {
			 x2=statek[k].pocisk_x[j]-1; y2=statek[k].pocisk_y[j]-5; s2=2; w2=10;}
		 if (statek[k].rodzaj_broni==2) {
			 x2=statek[k].pocisk_x[j]-2; y2=statek[k].pocisk_y[j]-2; s2=4; w2=4;}
	 
	if(x2>=x1 && x2<=x1+s1 && y2<=y1+w1 && y2>=y1 &&y2>=0) {

		if (statek[k].pocisk_strzelono[j]==0) continue; 
		if (statek[k].pocisk_strzelono[j]==1) {	//odejmowanie hp asteroidy i resetowanie pocisku
			statek[k].pocisk_strzelono[j]=0;
			statek[k].pocisk_x[j]=1000;
			if (statek[k].rodzaj_broni==1) {
				asteroida[i].hp-=10;
				//trafione[i]+=10;
				//ilosc_trafionych++;
				statek[k].punkty+=10;}//jeœli trafiono dodaje punkty nawet, gdy asteroida nie wybuch³a, ¿eby kilku graczy mog³o dostaæ punkty za trafienie
			if (statek[k].rodzaj_broni==2) {
				asteroida[i].hp-=5;
				//trafione[i]+=5;
				//ilosc_trafionych++;
				statek[k].punkty+=5;}
		}
	}
	 }
	}
	}
			/*los=rand()%100;				//10% szans na wylosowanie +50 punktow z zestrzelonej asteroidy
			if (los<10 && pkt50==0 && bron_znajdzka.czy_jest==0 && bomba.znajdzka==false && asteroida[i].hp<=0) {
				pkt50=1;
				pkt50_y=asteroida[i].y;
				pkt50_x=asteroida[i].x;
			}
			if (los>=10 && los<20 && pkt50==0 && bron_znajdzka.czy_jest==0 && bomba.znajdzka==false && asteroida[i].hp<=0) { //10% szans na wylosowanie broni
				bron_znajdzka.czy_jest=1;
				bron_znajdzka.x=asteroida[i].x;
				bron_znajdzka.y=asteroida[i].y;
			}

			if (los>=20 && los<25 && pkt50==0 && bron_znajdzka.czy_jest==0 && bomba.znajdzka==false && asteroida[i].hp<=0) { //5% szans na wylosowanie bomby
				bomba.znajdzka=true;
				bomba.x=asteroida[i].x;
				bomba.y=asteroida[i].y;
			}
		}}
	}*/
		/*			if (asteroida[i].hp<=0 && asteroida[i].wybuch<=20) {			//jesli udalo sie rozwalic asteroide...
					asteroida[i].wybuch++;
					//asteroida[i].x=-200;
					}
						if (asteroida[i].wybuch>20  && asteroida[i].x>-100) {
						//punkty+=asteroida[i].punkty;
						asteroida[i].x=-1000;
						ile_pozostalo--;}*/
	
	
}

//==================komunikacja poprzez protokó³ TCP=======================
int TCP() {
	struct in_addr adresIP ={0};
	char adresS[32];
	DWORD blad;

	//Wylosowanie danych asteroid, ¿eby ka¿dy gracz mia³ te same dane
	int los;
			for(i=0;i<50;i++) {
			los=(rand()%100)+1;
			if (los<=20) {//20% szansy na wylosowanie twardszych asteroid
				asteroida[i].lvl=2;
				asteroida[i].hp=40;
				}
			if (los>20 && los<=30) { //10% szansy na najtwardsze asteroidy
				asteroida[i].lvl=3;
				asteroida[i].hp=60;
			}
			if (los>30) { //70% szansy na wylosowanie zwyklych asteroid
			asteroida[i].lvl=1;
			asteroida[i].hp=20;
			}
			asteroida[i].x=rand()%500;
			asteroida[i].y=((rand()%100)-164)-(i*75);
			asteroida[i].rx=rand()%3-1;
			asteroida[i].ry=rand()%2+1;
			}

	//ustawianie statków, ¿eby wszystkie statki nie zaczyna³y w tym samym miejscu 
			statek[1].x=400;
			statek[1].y=550;
			statek[2].x=250;
			statek[2].y=550;
			statek[3].x=550;
			statek[3].y=550;

			for (i=1;i<=3;i++) {
				statek[i].punkty=0;
				statek[i].rodzaj_broni=1;
				statek[i].zniszczono=false;
				for (int j=0;j<20;j++) {
					statek[i].pocisk_x[j]=-100;
					statek[i].pocisk_y[j]=-100;
					statek[i].pocisk_strzelono[j]=false;
				}
			}

	
	printf("\n---Komunikacja przez TCP---\n");
	//strcpy(adres,"192.168.2.103");//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	//strcpy(adres,"127.0.0.1");
	printf("Podaj adres IP:\n");
	scanf("%s",&adres);
	printf("Adres IP serwera: %s\n",adres);
	adresIP.s_addr= inet_addr(adres);
	memcpy(adresS,&adresIP,4);
	host = gethostbyaddr(adresS,4,AF_INET);
	//---------------------------
		port=1234;
		printf("Port: %d\n",port);

	//---------------------------
    if (host == 0) {
		 blad = WSAGetLastError();
        if (blad != 0) {
            if (blad == WSAHOST_NOT_FOUND) {
                printf("Nie znaleziono hosta\n");
                return 1;
            } 
			else if (blad == WSANO_DATA) {
                printf("Nie znaleziono danych\n");
                return 1;
            }
			else {
                printf("Funkcja zwrocila blad\n");
                return 1;
			}
		}}
    else {
        printf("Nazwa: %s\n", host->h_name);
        for (pAlias = host->h_aliases; *pAlias != 0; pAlias++) {
            printf("Alternatywna nazwa %d: %s\n", ++i, *pAlias);
        }
	}

	SOCKET sock[4];
	for(i=0;i<=3;i++) sock[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKET acc[4];
	for(i=0;i<=3;i++) sock[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	int kod_bledu;

	for (i=1;i<=3;i++) {

		if (ilosc_SI !=0 && i < 3) continue;	//jeœli SI jest wlaczone, to pomija siê wysylanie danych do pierwszych 2 graczy(to beda SI), gracz ma nr 3

	SOCKADDR_IN addr;

	addr.sin_addr.S_un.S_addr=adresIP.s_addr;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
		puts("----------------------------------------");
	kod_bledu = bind(sock[i],(SOCKADDR*)&addr,sizeof(addr));
	if (kod_bledu==SOCKET_ERROR) {
		printf("Funckja bind() zwrocila blad nr %d\n",WSAGetLastError());
	}
	else puts("Funkcja bind() zadzialala prawidlowo");
	kod_bledu = listen(sock[i],SOMAXCONN);
	if (kod_bledu==SOCKET_ERROR) {
		printf("Funckja listen() zwrocila blad nr %d\n",WSAGetLastError());
	}
	else puts("Funkcja listen() zadzialala prawidlowo");

	if (ilosc_SI == 0 ) printf("Oczekiwanie na przylaczenie sie %d gracza\n",i);
	else printf("Oczekiwanie na przylaczenie sie gracza\n");

	int dl = sizeof(addr);
	acc[i]=accept(sock[i],(SOCKADDR*)&addr,(socklen_t *) & dl);
	if (acc[i] == INVALID_SOCKET) {
		printf("Funkcja accept() zwrocila blad %d\n",WSAGetLastError());
	}
	else puts("Zaakceptowano polaczenie...");

	kod_bledu=closesocket(sock[i]);	//ten socket jest ju¿ niepotrzebny, wiêc mo¿na go zamkn¹æ
	if (kod_bledu==SOCKET_ERROR) {
		printf("Funkcja closesocket() zwrocila blad nr %d\n",WSAGetLastError());
	}
	else puts("Zamknieto socket 'sock'");
	
	//odbieranie wiadomosci od gracza
	int dlugosc;
	sscanf(wiadomosc,"%d",&dlugosc);
	strcpy(wiadomosc,"");
		//sockaddr_in odbior;
		//int dlugosc = sizeof(odbior);
	kod_bledu=recv(acc[i],wiadomosc,5,NULL);
	if (kod_bledu > 0) 
		printf("Odebrano %d bajtow\n",kod_bledu);
	else if (kod_bledu==0) 
		printf("Polaczenie zamkniete\n");
	else puts("Nie odebrano wiadomosci");

	if (kod_bledu>0) {
		printf("Odebrano wiadomosc o przylaczeniu sie %d gracza\n",i);
		printf("Adres IP nadawcy: %s\n",inet_ntoa(addr.sin_addr));
		unsigned short port = ntohs(addr.sin_port);
		printf("Odebrano przez port: %d\n",port);
	}
	
	//wysylanie numeru gracza do gry
	char ktory_gracz[5];
	sprintf(ktory_gracz,"%d",i);
	kod_bledu = send (acc[i],ktory_gracz,5,NULL);
	if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad podczas wysylania wiadomosci nr %d\n",WSAGetLastError());
	}
	else printf("Wyslano potwierdzenie do %s gracza\n",ktory_gracz);
	//}

	char SI[5];
	sprintf(SI,"%d",ilosc_SI);
	kod_bledu = send (acc[i],ktory_gracz,5,NULL);
	if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad podczas wysylania wiadomosci nr %d\n",WSAGetLastError());
	}
	else printf("Wyslano ilosc SI do gracza\n",ktory_gracz);

	//wysylanie wspolrzednej x
	sprintf(wiadomosc,"%d",statek[i].x);
	kod_bledu = send (acc[i],wiadomosc,5,NULL);
	if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad podczas wysylania wiadomosci nr %d\n",WSAGetLastError());
	}
	else printf(">>Wyslano wspolrzedna x do %s gracza\n",ktory_gracz);

	//wysylanie wspolrzednej y
	sprintf(wiadomosc,"%d",statek[i].y);
	kod_bledu = send (acc[i],wiadomosc,5,NULL);
	if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad podczas wysylania wiadomosci nr %d\n",WSAGetLastError());
	}
	else printf(">>Wyslano wspolrzedna y do %s gracza\n",ktory_gracz);


	//Wysylanie danych asteroid
	for(int j=0;j<50;j++) {
		printf("%d Asteroida",j);
		//x
	sprintf(wiadomosc,"%d",asteroida[j].x);
	kod_bledu = send (acc[i],wiadomosc,5,NULL);
	if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad podczas wysylania wiadomosci nr %d\n",WSAGetLastError());
	}
	else printf(" x = %d",asteroida[j].x);
		//y
		sprintf(wiadomosc,"%d",asteroida[j].y);
	kod_bledu = send (acc[i],wiadomosc,5,NULL);
	if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad podczas wysylania wiadomosci nr %d\n",WSAGetLastError());
	}
	else printf(" y = %d",asteroida[j].y);
		//ruch po osi x
	sprintf(wiadomosc,"%d",asteroida[j].rx);
	kod_bledu = send (acc[i],wiadomosc,5,NULL);
	if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad podczas wysylania wiadomosci nr %d\n",WSAGetLastError());
	}
	else printf(" rx = %d",asteroida[j].rx);
		//ruch po osi y
	sprintf(wiadomosc,"%d",asteroida[j].ry);
	kod_bledu = send (acc[i],wiadomosc,5,NULL);
	if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad podczas wysylania wiadomosci nr %d\n",WSAGetLastError());
	}
	else printf(" ry = %d",asteroida[j].ry);
		//poziom asteroidy
	sprintf(wiadomosc,"%d",asteroida[j].lvl);
	kod_bledu = send (acc[i],wiadomosc,5,NULL);
	if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad podczas wysylania wiadomosci nr %d\n",WSAGetLastError());
	}
	else printf(" lvl = %d",asteroida[j].lvl);
		//"punkty ¿ycia" asteroid
	sprintf(wiadomosc,"%d",asteroida[j].hp);
	kod_bledu = send (acc[i],wiadomosc,5,NULL);
	if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad podczas wysylania wiadomosci nr %d\n",WSAGetLastError());
	}
	else printf(" hp = %d\n",asteroida[j].hp);
	}

	}

	//Wys³anie do graczy potwierdzenia, ¿e wszyscy gracze do³¹czyli do gry

	puts("------------------------------------------------\n");
	
	for (i=1;i<=3;i++) {

		if (ilosc_SI !=0 && i < 3) continue;

	strcpy(wiadomosc,"start");
	kod_bledu = send (acc[i],wiadomosc,5,NULL);
	if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad podczas wysylania wiadomosci nr %d\n",WSAGetLastError());
	}
	else printf(">>Rozpoczynam gre...\n");

		kod_bledu = shutdown(acc[i], SD_BOTH);
    if (kod_bledu == SOCKET_ERROR) {
        printf("Funkcja shutdown() zwrocila blad %d\n", WSAGetLastError());
        closesocket(sock[i]);
        WSACleanup();
        //return 1;
    }
	else puts("zamknieto polaczenie");

		kod_bledu=closesocket(acc[i]);
	if (kod_bledu==SOCKET_ERROR) {
	printf("Funkcja closesocket() zwrocila blad nr %d\n",WSAGetLastError());
	}
	else puts("Zamknieto socket acc");

	}

	puts(">>Zakonczono przyznawanie numerow gracza, nastapi uruchomienie komunikacji przez protokol UDP");

	WSACleanup();
	return 0;
}

//====================komunikacja poprzez protokó³ UDP=============================
int UDP() {
	struct in_addr adresIP ={0};
	char adresS[32];
	DWORD blad;

	puts("---------------------komunikacja przez UDP - statki----------------------------");
	//strcpy(adres,"192.168.2.103");//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	adresIP.s_addr= inet_addr(adres);
	memcpy(adresS,&adresIP,4);
	host = gethostbyaddr(adresS,4,AF_INET);
	//---------------------------
		//printf("Podaj port przez ktory chcesz odebrac wiadomosc: ");
		port=1234;
	//---------------------------
    if (host == 0) {
		 blad = WSAGetLastError();
        if (blad != 0) {
            if (blad == WSAHOST_NOT_FOUND) {
                printf("Nie znaleziono hosta\n");
                return 1;
            } 
			else if (blad == WSANO_DATA) {
                printf("Nie znaleziono danych\n");
                return 1;
            }
			else {
                printf("Funkcja zwrocila blad %d\n",blad);
                return 1;
			}
		}}
    else {
       /* printf("Nazwa: %s\n", host->h_name);
        for (pAlias = host->h_aliases; *pAlias != 0; pAlias++) {
            printf("Alternatywna nazwa %d: %s\n", ++i, *pAlias);
        }*/
	}




	SOCKET sock[4]; 
	for(i=1;i<=3;i++) {

	if (ilosc_SI != 0) {
		
		petla++;
		ruch_asteroid();		//"poruszanie" asteroidami, ¿eby SI wiedzia³y, gdzie siê znajduj¹
		funkcjaSI();	//jesli SI jest wlaczone, to trzeba uruchomic funkcje, ktora nimi porusza
		strzal(petla);
		kolizja_statku();
	}


		sock[i]=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		
	SOCKADDR_IN addr;

	addr.sin_addr.S_un.S_addr=adresIP.s_addr;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);

	int kod_bledu=bind(sock[i],(SOCKADDR*)&addr,sizeof(addr));
	if (kod_bledu==SOCKET_ERROR) {
		printf("funkcja bind() zwrocila blad %d\n",WSAGetLastError());
	}
	
		int liczba=0;
	puts("----------------------------------------------------");


	DWORD dwRecvfromTimeout = 1000;//timeout w milisekundach
		kod_bledu = setsockopt( sock[i], SOL_SOCKET, SO_RCVTIMEO, ( const char * ) &dwRecvfromTimeout, sizeof( dwRecvfromTimeout ) );
		if (kod_bledu == SOCKET_ERROR) {
			printf("setsockopt zwrocil blad nr %d\n",WSAGetLastError());
		}

				
		for (int j=1;j<=3;j++) {brak_polaczenia[j]++;}//jeœli serwer otrzyma dane od gracza, to zeruje odpowiedni¹ komórkê tablicy
				//dziêki temu mo¿na wykryæ to, ze gracz ju¿ nie gra lub straci³ po³¹czenie z serwerem
				
	//printf("Oczekiwanie na wiadomosc...\n");
		int dl = sizeof(addr);
	kod_bledu=recvfrom(sock[i],wiadomosc,15,NULL,(SOCKADDR*)&addr,(socklen_t *) & dl);	//odbieranie wiadomosci z danymi statku (który_gracz,x,y,punkty)
	if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad przy odebraniu wiadomosci %d\n",WSAGetLastError());
		odebrano=0;
		if (WSAGetLastError()==WSAETIMEDOUT && ilosc_SI!=0) brak_polaczenia[3]++;//Jeœli wystapil timeout
	}
	else {
		printf("Odebrano pakiet, zawartosc: %s\n",wiadomosc);
		odebrano=1;
	}
	
	if (odebrano) {
	char nr_gracza[1], x[4], y[4], str_punkty[4];
	int int_nr_gracza=0, int_x=0, int_y=0, int_punkty=0;
	
	nr_gracza[0]=wiadomosc[0];
	x[0]=wiadomosc[1];
	x[1]=wiadomosc[2];
	x[2]=wiadomosc[3];
	x[3]=wiadomosc[4];
	y[0]=wiadomosc[5];
	y[1]=wiadomosc[6];
	y[2]=wiadomosc[7];
	y[3]=wiadomosc[8];
	str_punkty[0]=wiadomosc[9];
	str_punkty[1]=wiadomosc[10];
	str_punkty[2]=wiadomosc[11];
	str_punkty[3]=wiadomosc[12];

	int_nr_gracza=atoi(nr_gracza);
	int_x=atoi(x);
	int_y=atoi(y);
	int_punkty=atoi(str_punkty);

	brak_polaczenia[int_nr_gracza]=0;/////////////////////

	char zestrzelono = 'a';//przypisanie takiej wartosci, ¿eby zmienna nie zainicjalizowala sie z wartoscia '0'
	zestrzelono = wiadomosc[14];
	if (zestrzelono=='0') {
		zestrzelono_wszystkie_asteroidy=true;
		printf("Gracz %d zasygnalizowal, ze wszystkie asteroidy zostaly zniszczone",int_nr_gracza);
	}

	//printf("Zapisano jako: %d %d %d\n",int_nr_gracza,int_x,int_y);

	statek[int_nr_gracza].x=int_x;
	statek[int_nr_gracza].y=int_y;
	statek[int_nr_gracza].punkty=int_punkty;

		int gracz[2];
		if (int_nr_gracza==1) {
			gracz[0]=2;
			gracz[1]=3;}
		else if (int_nr_gracza==2) {
			gracz[0]=1;
			gracz[1]=3; }
		else {
			gracz[0]=1;
			gracz[1]=2; }


	for (int j=0;j<2;j++) { //wysylanie dwukrotnie danych innych graczy - np do pierwszego gracza wysyla sie dane graczy 2 i 3
		char nr_gracza[1], x[4], y[4], punkty[4];
		sprintf(nr_gracza,"%1d",gracz[j]);
		sprintf(x,"%4d",statek[gracz[j]].x);
		sprintf(y,"%4d",statek[gracz[j]].y);
		sprintf(y,"%4d",statek[gracz[j]].y);//jak siê to wpisa³o 1 raz, to wrzuca³o do zmiennej wiadomosc 2 razy to samo
											//gdy 2 razy urochomi siê sprintf, to wpisuje bez dublowania

	strcpy(wiadomosc,nr_gracza);
	strcat(wiadomosc,x);
	strcat(wiadomosc,y);

	sprintf(punkty,"%4d",statek[gracz[j]].punkty);
	//printf("Punkty: %s\n",punkty);
	//strcat(wiadomosc,str_punkty);
	wiadomosc[9]=punkty[0];
	wiadomosc[10]=punkty[1];
	wiadomosc[11]=punkty[2];
	wiadomosc[12]=punkty[3];

	kod_bledu = sendto (sock[i],wiadomosc,15,NULL,(SOCKADDR*)&addr,sizeof(addr));
		if (kod_bledu==SOCKET_ERROR) {
			printf("%d petla, Wystapil blad przy wysylaniu wiadomosci %d\n",j,WSAGetLastError());
			//polaczenie(i,false);
		}
		else printf("Wyslano wiadomosc %s\n",wiadomosc);

	}
	}
	
	//Obsluga straconych polaczen//////////////////
	if (ilosc_SI != 0 && brak_polaczenia[3]>=10) {
		printf("************************************\n");
		printf("**Stracono polaczenie z %d graczem***\n",3);
		printf("************************************\n");

		DisplayResourceNAMessageBox(1);
		return -1;
	}

	int ile_zerwanych_polaczen=0;
	for (int j=1;j<=3;j++) {
		if (brak_polaczenia[j]>=10 && ilosc_SI==0) {
			ile_zerwanych_polaczen++;
			printf("\n****Stracono polaczenie z %d graczem****\n\n",j);
		}
	}
	if (ile_zerwanych_polaczen==3 && ilosc_SI==0) {
		printf("\n\n        Stracono polaczenie ze wszystkimi graczami\n               Serwer zostanie wylaczony\n\n\n");
		DisplayResourceNAMessageBox(3);
		return -1;
	}

	
	closesocket(sock[i]);
	//puts("Zamknieto socket");

	UDP_asteroidy();/////////////////////////////////////////	

	}

	return 0;
}

//====================komunikacja poprzez protokó³ UDP (2)===========================
int UDP_asteroidy() {
	struct in_addr adresIP ={0};
	char adresS[32];
	DWORD blad;

	puts("-----------komunikacja przez UDP - asteroidy-----------");
	//strcpy(adres,"192.168.2.103");//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	adresIP.s_addr= inet_addr(adres);
	memcpy(adresS,&adresIP,4);
	host = gethostbyaddr(adresS,4,AF_INET);
	//---------------------------
		//printf("Podaj port przez ktory chcesz odebrac wiadomosc: ");
		port=1234;
	//---------------------------
    if (host == 0) {
		 blad = WSAGetLastError();
        if (blad != 0) {
            if (blad == WSAHOST_NOT_FOUND) {
                printf("Nie znaleziono hosta\n");
                return 1;
            } 
			else if (blad == WSANO_DATA) {
                printf("Nie znaleziono danych\n");
                return 1;
            }
			else {
                printf("Funkcja zwrocila blad %d\n",blad);
                return 1;
			}
		}}


	SOCKET sock; 
		sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	SOCKADDR_IN addr;

	addr.sin_addr.S_un.S_addr=adresIP.s_addr;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);

	kod_bledu=bind(sock,(SOCKADDR*)&addr,sizeof(addr));
	if (kod_bledu==SOCKET_ERROR) {
		printf("funkcja bind() zwrocila blad %d\n",WSAGetLastError());
	}
	
	
	int dl = sizeof(addr);
	int odebrano=0;
	
		timeval t;
		t.tv_sec=0;
		t.tv_usec=500;//timeout w mikrosekundach
		DWORD dwRecvfromTimeout = 10;//timeout w milisekundach
		kod_bledu = setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, ( const char * ) &dwRecvfromTimeout, sizeof( dwRecvfromTimeout ) );
		if (kod_bledu == SOCKET_ERROR) {
			printf("setsockopt zwrocil blad nr %d\n",WSAGetLastError());
		}
		
	//odebranie iloœci trafionych asteroid przez gracza
	kod_bledu=recvfrom(sock,wiadomosc,2,NULL,(SOCKADDR*)&addr,(socklen_t *) & dl);
	if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad przy odebraniu wiadomosci %d\n",WSAGetLastError());
		odebrano=0;
	}
	else {
		printf("Odebrano pakiet, zawartosc: %s\n",wiadomosc);
		odebrano = 1;
	}
	
	if (!odebrano) puts("Nie udalo sie odebrac wiadomosci");
	else {

	int ilosc_trafionych=0;
	ilosc_trafionych=atoi(wiadomosc);


	int hp_odjete=0;
	int ktora=0;
	
	if (ilosc_trafionych>0) {
	for (int j=0;j<ilosc_trafionych;j++) {
		dl=sizeof(addr);
		kod_bledu=recvfrom(sock,wiadomosc,3,NULL,(SOCKADDR*)&addr,(socklen_t *) & dl);	//ktora asteroida zostala trafiona
		if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad przy odebraniu wiadomosci %d\n",WSAGetLastError());
		}
		else printf("Odebrano pakiet, zawartosc: %s\n",wiadomosc);

		ktora=atoi(wiadomosc);//która asteroida zosta³a trafiona
		if (ktora >= 50) break;

		kod_bledu=recvfrom(sock,wiadomosc,3,NULL,(SOCKADDR*)&addr,(socklen_t *) & dl); //ile hp nale¿y odj¹æ
		if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad przy odebraniu wiadomosci %d\n",WSAGetLastError());
		}
		else printf("Odebrano pakiet, zawartosc: %s\n",wiadomosc);
		hp_odjete=atoi(wiadomosc);
		asteroida[ktora].hp-=hp_odjete;//odejmowanie hp
	}
	}

	/*if (ilosc_asteroid >0 )
	for (int j=0;j<10;j++) { //sprawdzenie, czy asteroida zosta³a zestrzelona
		if (asteroida[j].hp <=0 && asteroida[j].hp>-50) {
			ilosc_asteroid--;
			asteroida[j].hp=-100;
		}
	}
	else for (int j=0;j<10;j++) {
		if (asteroida[i].lvl == 1) asteroida[j].hp=10;
		else if (asteroida[j].lvl == 2) asteroida[j].hp=20;
		else asteroida[j].hp=30;
	}
	*/
	dl=sizeof(addr);
	for (int j=0;j<10;j++) {	
		//wysylanie punktow zycia asteroid do gracza
		sprintf(wiadomosc,"%d",asteroida[j].hp);
		kod_bledu = sendto (sock,wiadomosc,3,NULL,(SOCKADDR*)&addr,sizeof(addr));
		if (kod_bledu==SOCKET_ERROR) {
			printf("Wystapil blad przy wysylaniu wiadomosci %d\n",WSAGetLastError());
			//getchar();
			break;
		}
		else printf("a%d: %s ",j,wiadomosc);
		//if (j == 9) puts(" ");

		if (ilosc_SI!=0) {
		//odebranie wspolrzednej x od gracza
		kod_bledu=recvfrom(sock,wiadomosc,5,NULL,(SOCKADDR*)&addr,(socklen_t *) & dl);	//ktora asteroida zostala trafiona
		if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad przy odebraniu wiadomosci %d\n",WSAGetLastError());
		}
		else printf("Odebrano pakiet, zawartosc: %s\n",wiadomosc);

		asteroida[j].x=atoi(wiadomosc);
		
		//odebranie wspolrzednej y
		kod_bledu=recvfrom(sock,wiadomosc,5,NULL,(SOCKADDR*)&addr,(socklen_t *) & dl);	//ktora asteroida zostala trafiona
		if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad przy odebraniu wiadomosci %d\n",WSAGetLastError());
		}
		else printf("Odebrano pakiet, zawartosc: %s\n",wiadomosc);

		asteroida[j].y=atoi(wiadomosc);
		
		if (j == 9) puts(" ");

		}
	}

	
	//if (ilosc_SI !=0 ) {
	for (i=1;i<=2;i++) {
		for (int j=0;j<20;j++) {	//wysylanie danych pocisków
			sprintf(wiadomosc,"%d",statek[i].pocisk_x[j]);
			kod_bledu = sendto (sock,wiadomosc,3,NULL,(SOCKADDR*)&addr,sizeof(addr));
			if (kod_bledu==SOCKET_ERROR) {
			printf("Wystapil blad przy wysylaniu wiadomosci %d\n",WSAGetLastError());
			//getchar();
			break;
		}
		//else printf("p%dx: %s ",j,wiadomosc);
		//if (j == 5) puts(" ");

			sprintf(wiadomosc,"%d",statek[i].pocisk_y[j]);
			kod_bledu = sendto (sock,wiadomosc,3,NULL,(SOCKADDR*)&addr,sizeof(addr));
			if (kod_bledu==SOCKET_ERROR) {
			printf("Wystapil blad przy wysylaniu wiadomosci %d\n",WSAGetLastError());
			//getchar();
			break;
		}
		//else printf("p%dy: %s ",j,wiadomosc);
		//if (j == 5) puts(" ");
	
		sprintf(wiadomosc,"%d",statek[i].pocisk_strzelono[j]);
			kod_bledu = sendto (sock,wiadomosc,3,NULL,(SOCKADDR*)&addr,sizeof(addr));
			if (kod_bledu==SOCKET_ERROR) {
			printf("Wystapil blad przy wysylaniu wiadomosci %d\n",WSAGetLastError());
			//getchar();
			break;
		}
		//else printf("p%ds: %s ",j,wiadomosc);
		//if (j%4==0) puts(" ");
	}
	}
	//}
	
	}//odebrano

	closesocket(sock);

	return 0;
}

//=============================SI===============================
void funkcjaSI () {//sztuczna, pseudolosowa "inteligencja"

	srand (time(NULL));

	SI[1].poprzedni_ruch_x=SI[1].ruch_x;
	SI[1].poprzedni_ruch_y=SI[1].ruch_y;
	SI[2].poprzedni_ruch_x=SI[2].ruch_x;
	SI[2].poprzedni_ruch_y=SI[2].ruch_y;

	int los;
	
	for (i=1;i<=2;i++) {	//sprawdzenie, czy w pobli¿u s¹ asteroidy, jeœli tak to statki kierowane przez komputer powinny siê oddalaæ
		if (statek[i].zniszczono==false) {
			double odleglosc=0; 
			bool asteroidy_w_poblizu=false;
				double sx=double(statek[i].x);
				double sy=double(statek[i].y);

			for (int j=0;j<10;j++) {
				if (asteroida[i].hp > 0) {
				double ax=double(asteroida[j].x/*+32*/);
				double ay=double(asteroida[j].y/*+32*/);

				odleglosc = sqrt ( double(abs((ax-sx) * (ax-sx))) + double(abs((ay-sy) * (ay-sy))) );
				
				if (odleglosc <= 150 /*&& statek[i].x>=24 && statek[i].x<=576 && statek[i].y>=24 && statek[i].y<=576*/) {
					if (ax<=sx) statek[i].x+=5;
					else statek[i].x-=5;
					if (ay<=ax) statek[i].y+=5;
					else statek[i].y-=5;
				}
				}
			}
	

		//for (i=1;i<=2;i++) {		//unikanie zderzenia z innymi statkami
			for (int j=1;j<=3;j++) {
				if (i != j) {
				double odleglosc=0;
				double s1x=statek[i].x;
				double s1y=statek[i].y;
				double s2x=statek[j].x;
				double s2y=statek[j].y;

				odleglosc = sqrt ( double( abs( (s1x-s2x) * (s1x-s2x) )) + double( abs( (s1y-s2y) * (s1y-s2y) )) );

				if (odleglosc <= 75 ) {
					if (s1x<s2x) {
						statek[i].x-=2;
						statek[j].x+=2;
					}
					else {
						statek[i].x+=2;
						statek[j].x+=2;
					}
					if (s1y<s2y) {
						statek[i].y-=2;
						statek[j].y+=2;
					}
					else {
						statek[i].y+=2;
						statek[j].y-=2;
					}
				}
				}
			}
		//}

		if (!asteroidy_w_poblizu) {	//gdy w pobli¿u nie znajduj¹ siê asteroidy
			if (SI[i].poprzedni_ruch_x != 0) {
				los = rand()%100;	//wylosowanie szansy na ruch w procentach 
				if (los<90)			//90%szansy na to, ¿e SI powtorzy ostatni ruch
					SI[i].ruch_x=SI[i].poprzedni_ruch_x;
				else
					SI[i].ruch_x=(rand()%15)-7;
			}
				else
					SI[i].ruch_x=(rand()%9)-4;

			if (statek[i].x<24 && SI[i].ruch_x<0 || statek[i].x>576 && SI[i].ruch_x>0) SI[i].ruch_x*=-1;	//gdy SI chce wylecieæ poza planszê, to trzeba odwróciæ ruch w tej wspó³rzêdnej	
	
			if (SI[i].poprzedni_ruch_y != 0) {
				los = rand()%100;	//wylosowanie szansy na ruch w procentach 
				if (los<90)			//90%szansy na to, ¿e SI powtorzy ostatni ruch
					SI[i].ruch_y=SI[i].poprzedni_ruch_y;
				else 
					SI[i].ruch_y=(rand()%15)-7;
			}
				else
					SI[i].ruch_y=(rand()%9)-4;
		}

		if (statek[i].y<24 && SI[i].ruch_y<0 || statek[i].y>576 && SI[i].ruch_y>0) SI[i].ruch_y*=-1;	//gdy SI chce wylecieæ poza planszê, to trzeba odwróciæ ruch w tej wspó³rzêdnej	
	

	statek[1].x+=SI[1].ruch_x;
	statek[1].y+=SI[1].ruch_y;
	statek[2].x+=SI[2].ruch_x;
	statek[2].y+=SI[2].ruch_y;
	}
	}
}

//================main============================
int main(int argc, wchar_t **argv) {

	WSAStartup();

	puts("-------------------------------------------------------------------------------");
	printf("\t\t\tSpace shooter v1.0 Server\n");
	printf("Politechnika Swietokrzyska w Kielcach\n");
	printf("Autorzy: Rafal Bebenek i Adrian Bartosinski\nInformatyka rok 2012/2013, grupa 211A\n");
	puts("-------------------------------------------------------------------------------");

	printf("Wybierz odpowiednia opcje:\n0. SI wylaczone (3 graczy)\n1. SI wlaczone (2 przeciwnikow, 1 gracz)\n");
	scanf("%d",&ilosc_SI);

	TCP();

	WSAStartup();
	for(;;) {
		/*if (zestrzelono_wszystkie_asteroidy) {
			zestrzelono_wszystkie_asteroidy=false;
			TCP();
			WSAStartup();
		}*/
	int kod=UDP();
	if (kod==-1) break;
	}
	//getchar();
	getchar();

	return 0;
}