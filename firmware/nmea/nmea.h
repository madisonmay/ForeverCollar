#include <avr/io.h>
#include <stdlib.h>
#include <string.h>

//sample nmea strings for testing parser
// double lat = 42.292747;
// double lng = -71.264622;
double lat = 43.0;
double lng = -73.0;
double max_dist = .200;

typedef struct _latlng {
  double lat;
  double lng;
  double dist;
  uint8_t valid;
  uint8_t roaming;
  char* sms;
} latlng;

void parse_nmea(char* buff, latlng* gps);
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
  uint8_t i=0; // used to iterate through array
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
  //error char at index 1
  //longitude should be stored at index 2
  //latitude should be stord at index 4

  //indices will have to be changed if our gps module
  //speaks a different dialog of NMEA

  char* valid = token[1]; //A --> Valid, V --> Invalid
  char* lat_str = token[2]; //longitude
  char* lng_str = token[4]; //latitude

  gps->valid = *valid;

  if (*valid == 'A') {
    gps->valid = 1;
    char* string_rep = concat(lat_str, concat(" ", lng_str));
    send_string(string_rep);
    //converts string stored in gps->lat_str to double and stores in lat
    gps->lat = atof(lat_str)/100.;

    //converts string stored in gps->lng_str to double and stores in lng
    gps->lng = atof(lng_str)/100.;
    gps->dist = distance(gps->lat, gps->lng);
    if (gps->dist > max_dist) {
      gps->roaming = 1;
    } else {
      gps->roaming = 0;
    }
  } else {
    gps->valid = 0;
    send_string("No GPS Signal");
  }


}


void parse_nmea(char* buff, latlng* gps) {

    //parse lat and lng out of raw nmea string
    parse_nmea_string(buff, gps);
    // send_string("Parsed");

    if (gps->dist > max_dist) {

      char lat_buff[50];
      char lng_buff[50];
      char dist_buff[50];

      sprintf(lat_buff, "%f", gps->lat);
      sprintf(lng_buff, "%f", gps->lng);
      sprintf(dist_buff, "%f", gps->dist);

      /* Debugging code */
      //determine how many characters are in the gps latitude and longitude strings
      // int len_lat = strlen(lat_buff);
      // int len_lng = strlen(lng_buff);
      // int len_dist = strlen(dist_buff);

      // //iterate over lat and lng strings, sending them char by char over usb
      // for (int i=0; i<len_lat; i++) {
      //   send_byte(lat_buff[i]);  
      // }

      // //lat/lng seperator 
      // send_byte(' ');  
      // for (int j=0; j<len_lng; j++) {
      //   send_byte(lng_buff[j]);  
      // }

      //       //lat/lng seperator 
      // send_byte(' ');  
      // for (int k=0; k<len_dist; k++) {
      //   send_byte(dist_buff[k]);  
      // }

      char* lat_str = &lat_buff[0];
      char* lng_str = &lng_buff[0];
      gps->sms = concat(concat(lat_str, ","), lng_str);
      // send_string(gps->sms);
    }

    //presumably has to do with memory management
    //for now, just make sure to call it after you're done sending bytes over usb
    // break_and_flush();
}
