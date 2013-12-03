#include <avr/io.h>
#include <stdlib.h>
#include <string.h>

//sample nmea strings for testing parser
double lat = 42.292747;
double lng = -71.264622;
double max_dist = .200;

typedef struct _latlng {
  double lat;
  double lng;
} latlng;

char* parse_nmea(void);
extern void parse_nmea_string(char *s, latlng *gps);
double distance(double gpslat, double gpslng);

#define d2r (M_PI / 180.0)

//calculate distance, assuming earth is spherical
double distance(double gpslat, double gpslng) {
    double dlong = (gpslng - lng) * d2r;
    double dlat = (gpslat - lat) * d2r;
    double a = pow(sin(dlat/2.0), 2) + cos(lat*d2r) * cos(gpslat*d2r) * pow(sin(dlong/2.0), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    double d = 6367 * c;

    return d;
}



void parse_nmea_string(char *s, latlng *gps)
{
  int i=0; // used to iterate through array
  char *token[20]; //stores the chunks of string after splitting the string on commas

  token[0] = strtok(s, ","); //get pointer to first token found and store in
                             //first element of array
  while(token[i] != NULL) {  //while commas continue to be found
      i++;  
      token[i] = strtok(NULL, ","); //continue to split the string
  }

  //Example: token = [], s = "a,b,c"
  //Iteration 1
  //token --> ["a"], s-->"b,c"
  //token --> ["a", "b"], s-->"c"
  //token --> ["a", "b", "c"], s-->""
  //end

  //when parsing GPRMC data
  //longitude should be stored at index 3
  //latitude should be stord at index 5

  //indices will have to be changed if our gps module
  //speaks a different dialog of NMEA

  char* lat_str = token[3]; //longitude
  char* lng_str = token[5]; //latitude

  //converts string stored in gps->lat_str to double and stores in lat
  gps->lat = atof(lat_str)/100.;

  //converts string stored in gps->lng_str to double and stores in lng
  gps->lng = atof(lng_str)/100.;
}


char* parse_nmea(void) {
 
    char *buff = "$GPRMC,71.132,A,4230.00,N,-7130.00,E,11.2,0.0,261206,0.0,E*50\r\n";

    //latlng struct to store gps data in
    //reused in every iterationif (d
  
    latlng gps;

    //parse lat and lng out of raw nmea string
    parse_nmea_string(buff, &gps);

    double dist = distance(gps.lat, gps.lng);

    char** result = malloc(30 * sizeof(char*));

    if (dist > max_dist) {

      char lat_buff[100];
      char lng_buff[100];
      char dist_buff[100];

      sprintf(lat_buff, "%f", gps.lat);
      sprintf(lng_buff, "%f", gps.lng);
      sprintf(dist_buff, "%f", dist);

      //determine how many characters are in the gps latitude and longitude strings
      int len_lat = strlen(lat_buff);
      int len_lng = strlen(lng_buff);
      int len_dist = strlen(dist_buff);

      //iterate over lat and lng strings, sending them char by char over usb
      for (int i=0; i<len_lat; i++) {
        send_byte(lat_buff[i]);  
      }

      //lat/lng seperator 
      send_byte(' ');  
      for (int j=0; j<len_lng; j++) {
        send_byte(lng_buff[j]);  
      }

            //lat/lng seperator 
      send_byte(' ');  
      for (int k=0; k<len_dist; k++) {
        send_byte(dist_buff[k]);  
      }

      char* lat_str = &lat_buff[0];
      char* lng_str = &lng_buff[0];
      *result = concat(concat(lat_str, ","), lng_str);
    }

    //presumably has to do with memory management
    //for now, just make sure to call if after you're done sending bytes over usb
    break_and_flush();
    return *result;
}
