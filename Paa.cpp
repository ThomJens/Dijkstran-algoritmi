// Thomas Jensen
// thomasjensen@kamk.fi
#include <iostream>
#include <vector>
#include <sstream>
#include <thread>
#include <future>
#include <fstream>
using namespace std;

class Solu {

	private:

		int tunnus;
		int arvo;
		vector <Solu*> neighbours;

	public:

		void maaritaTunnus(int nimi) {
			tunnus = nimi;
		}

		void maaritaNaapuri(Solu* naapuri) {
			neighbours.push_back(naapuri);
		}

		void maaritaArvo(int arvo) {
			arvo = arvo;
		}

		int tulostaTunnus() {
			return tunnus;
		}

		int tulostaArvo() {
			return arvo;
		}

		vector <Solu*> tulostaNaapurit() {
			return neighbours;
		}

		Solu* tulostaNaapuri(int naapuri) {
			return neighbours[naapuri];
		}

		struct Solu(int t, int a) {
			tunnus = t;
			arvo = a;
		};

};


vector <vector <Solu*>> ruudukko(int leveys, int korkeus, vector <Solu*> soluKokoelma);

int lyhyinMatka(vector <int> matka, vector <bool> kaytyKokoelma);

vector <Solu*> dijkstranAlgoritmi(vector <vector <int>> matriisi, int verteksiMaara, int alku, int loppu, vector <Solu*> kokoelma);


int main() {

	int leveys = 10;
	int korkeus = 10;
	int maara = leveys * korkeus;
	int alku = 0;
	int loppu = maara - 1;
	ofstream img("kuva.ppm");
	int korkeusKerroin = 1000 / korkeus;
	int leveysKerroin = 1000 / leveys;
	img << "P3" << endl;
	img << leveys * leveysKerroin << " " << korkeus * korkeusKerroin << endl;
	img << "255" << endl;
	int r, g, b;
	vector <vector <int> > dijkMatriisi(maara, vector <int> (maara, 0));
	vector <Solu*> naapurit;						// Solusta tehtiin pointeri, muuten ei olisi toiminut

	srand(time(NULL));

	for (int x = 0; x < maara; x++) {		
		naapurit.push_back(new Solu(x, rand() % 10 + 1));				//muista poistaa oliot muistista
	}

	vector <vector <Solu*>> soluMatriisi = ruudukko(leveys, korkeus, naapurit);

	//Selvitet‰‰n mitk‰ solut ovat toisilleen naapureita.
	int viereiset[8][2] = { {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1} };
	for (int rivi = 0; rivi < soluMatriisi.size(); rivi++) {
		for (int sarake = 0; sarake < soluMatriisi[0].size(); sarake++) {
			for (int vier = 0; vier < 8; vier++) {
				int naapuriX = rivi + viereiset[vier][0];
				int naapuriY = sarake + viereiset[vier][1];
				if (naapuriX >= 0 && naapuriX < soluMatriisi.size() && naapuriY >= 0 && naapuriY < soluMatriisi[0].size()) {
					soluMatriisi[rivi][sarake]->maaritaNaapuri(soluMatriisi[naapuriX][naapuriY]);
				}
			}
		}
	}

	//Luodaan matriisi, johon sijoitetaan solujen arvot, eli reitti painotukset.
	for (int rivi = 0; rivi < soluMatriisi.size(); rivi++) {
		dijkMatriisi[rivi][rivi] = 0;
		for (int sarake = 0; sarake < soluMatriisi[0].size(); sarake++) {
			vector <Solu*> vektori = soluMatriisi[rivi][sarake]->tulostaNaapurit();
			for (int v = 0; v < vektori.size(); v++) {
				dijkMatriisi[soluMatriisi[rivi][sarake]->tulostaTunnus()][vektori[v]->tulostaTunnus()] = vektori[v]->tulostaArvo();
			}
		}
	}

	auto future = async(dijkstranAlgoritmi, dijkMatriisi, maara, alku, loppu, naapurit);
	vector <Solu*> reitti = future.get();

	//Rakennetaan PBM tiedosto.
	for (int y = 0; y < korkeus * korkeusKerroin; y++) {
		for (int x = 0; x < leveys * leveysKerroin; x++) {
			int soluTunnus = soluMatriisi[y / korkeusKerroin][x / leveysKerroin]->tulostaTunnus();
			for (int z = 0; z < reitti.size(); z++) {
				if (soluTunnus == reitti[z]->tulostaTunnus() || soluTunnus == alku || soluTunnus == loppu) {
					r = 150;
					g = 0;
					b = 0;
					break;
				}
				else {
					r = 100 + soluMatriisi[y / korkeusKerroin][x / leveysKerroin]->tulostaArvo() * 15;
					g = 100 + soluMatriisi[y / korkeusKerroin][x / leveysKerroin]->tulostaArvo() * 15;
					b = 100 + soluMatriisi[y / korkeusKerroin][x / leveysKerroin]->tulostaArvo() * 15;
				}
			}
			img << r << " " << g << " " << b << " " << endl;
		}
	}

	#if _WIN32
		system("explorer kuva.ppm");
	#elif __LINUX__
		system("explorer kuva.ppm");
	#elif __APPLE__
		system("open kuva.ppm");
	#endif
	
	naapurit.erase(naapurit.begin(), naapurit.end());
	for (int m = 0; m < soluMatriisi.size(); m++) {
		soluMatriisi[m].erase(soluMatriisi[m].begin(), soluMatriisi[m].end());
	}
	reitti.erase(reitti.begin(), reitti.end());

	return 0;
}

//Luodaan matriisi, johon sijoitetaan luodut solut.
vector <vector <Solu*>> ruudukko(int leveys, int korkeus, vector <Solu*> soluKokoelma) {
	vector <vector <Solu*>> matriisi(leveys, vector <Solu*>(korkeus));
	for (int sl = 0; sl < leveys; sl++) {
		for (int sk = 0; sk < korkeus; sk++) {
			int lkSijainti = sk + sl * korkeus;
			matriisi[sl][sk] = soluKokoelma[lkSijainti];
		}
	}

	return matriisi;
}

//Palautetaan pienin arvo soluista joihin ei ole viel‰ menty.
int lyhyinMatka(vector <int> matka, vector <bool> kaytyKokoelma) {
	int minimi = INT_MAX, indeksi;
	for (unsigned int k = 0; k < matka.size(); k++) {

		if (kaytyKokoelma[k] == false && matka[k] <= minimi) {
			minimi = matka[k];
			indeksi = k;
		}
	}
	return indeksi;
}
 
vector <Solu*> dijkstranAlgoritmi(vector <vector <int>> matriisi, int verteksiMaara, int alku, int loppu, vector <Solu*> soluKokoelma) {
	vector <int> etaisyys(verteksiMaara, INT_MAX);
	vector <bool> kaytySetti(verteksiMaara, false);
	vector <vector <Solu*>> solut(verteksiMaara);
	
	//K‰yd‰‰n jokainen piste l‰pi ja lasketaan sille lyhin reitti jokaiselle pisteelle.
	etaisyys[alku] = 0;
	for (unsigned int x = 0; x < verteksiMaara; x++)
	{
		int m = lyhyinMatka(etaisyys, kaytySetti);
		kaytySetti[m] = true;
		for (unsigned int k = 0; k < verteksiMaara; k++)
		{
			if (!kaytySetti[k] && matriisi[m][k] && etaisyys[m] != INT_MAX && etaisyys[m] + matriisi[m][k] < etaisyys[k]) {
				etaisyys[k] = etaisyys[m] + matriisi[m][k];
				solut[k].push_back(soluKokoelma[m]);
			};
		}
	}

	vector <Solu*> reitti;
	Solu* loppuPiste = solut[loppu][0];
	Solu* alkuPiste = soluKokoelma[alku];

	// Rakennetaan lyhin reitti alku ja loppu pisteen v‰lille.
	do {
		for (int k = 0; k < solut.size(); k++) {
			for (int s = 0; s < solut[k].size(); s++) {
				int loppuTunnus = loppuPiste->tulostaTunnus();
				if (loppuTunnus == solut[k][s]->tulostaTunnus()) {
					if (loppuTunnus > 0) {
						reitti.push_back(loppuPiste);;
						if (loppuPiste->tulostaTunnus() == alku) {
							break;	
						}
						else {
							loppuPiste = solut[loppuTunnus][0];
						}
					}
					break;
				}
			}
		}
	} while (loppuPiste->tulostaTunnus() != alkuPiste->tulostaTunnus());
	return reitti;
}