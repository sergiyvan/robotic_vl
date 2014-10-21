/**
 * @file
 *
 * Dies ist die Beschreibung des headers.
 *
 * Der erste Satz ist eine kurze Beschreibung. Alle folgenden Saetze sind
 * eher eine Detailbeschreibung. Das _at_file am Anfang des Kommentars ist
 * wichtig. Je nach Datei ist eine Dateibeschreibung nicht noetig, und eine
 * Klassenbeschreibung reicht aus.
 *
 * Ersten Satz von der Detailbeschreibung am besten mit einem Absatz trennen.
 *
 * Der Autor muss nicht angegeben werden, kann aber das _at_Zeichen nicht
 * vergessen.
 *
 * @author stefan
 */

#ifndef TEST1_H_
#define TEST1_H_


/*------------------------------------------------------------------------------------------------*/

/**
 * Hier wird die Klasse beschrieben.
 *
 * Ersten Satz (Kurzbeschreibung) von der restlichen Detailbeschreibung durch
 * Absatz trennen.
 *
 * JEDE Klasse sollte ueber Dokumentation verfuegen.
 *
 * Funktionen werden am besten da dokumentiert, wo sie auch implementiert
 * werden (i.A. in der cpp). Sofern die Semantik der Funktion klar ist, z.B.
 * getter und setter (aber auch da nicht immer), muss nicht dokumentiert
 * werden.
  */
class DoxygenDemo {
private:
	int counter;

public:
	DoxygenDemo();
	// hier ist z.B. keine doxygen docu noetig
	virtual ~DoxygenDemo() {};

	void print();
};

#endif /* TEST1_H_ */
