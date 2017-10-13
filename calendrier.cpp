#include "calendrier.h"

Calendrier::Calendrier(int time)
{
    timeBetweenSeasons = time;
    seasonTimer.start();
}

void Calendrier::updateCalendrier()
{
    if(float(seasonTimer.elapsed()) / 1000 > timeBetweenSeasons)
    {
        emit changeSeason();
        seasonTimer.restart();
    }
}
