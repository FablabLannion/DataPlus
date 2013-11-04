/*********************************************************************
 * Data+ AquaMarium project
 * Fablab Lannion 
 * http://fablab-lannion.org/wiki/index.php?title=AquaMarium
 *********************************************************************
 * Copyright: (c) 2013 Jérôme Labidurie
 * Licence:   GNU General Public Licence version 3
 * Email:     jerome.labidurie at gmail.com
 *********************************************************************
 * This file is part of AquaMarium.
 * 
 * AquaMarium is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AquaMarium is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with AquaMarium.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************
 * Needed Librairies :
 * - AccelStepper : http://www.airspayce.com/mikem/arduino/AccelStepper/
 *********************************************************************
 */

#include <AccelStepper.h>

// Define a stepper and the pins it will use
AccelStepper stepper(AccelStepper::DRIVER, 9, 8);

int pos = 3600;

void setup()
{
   stepper.setMaxSpeed(3000);
   stepper.setAcceleration(1000);
}


void loop()
{
   // test stepper control
   if (stepper.distanceToGo() == 0)
   {
      delay(500);
      pos = -pos;
      stepper.moveTo(pos);
   }
   stepper.run();
}
