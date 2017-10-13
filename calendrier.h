#ifndef CALENDRIER_H
#define CALENDRIER_H

#include <QElapsedTimer>
#include <QObject>

class Calendrier: public QObject
{
    Q_OBJECT

public:
    Calendrier(int timeBetweenSeasons);


signals:
    void changeSeason();

public slots:
    void updateCalendrier();

private :
    int timeBetweenSeasons;
    QElapsedTimer seasonTimer;

};

#endif // CALENDRIER_H
