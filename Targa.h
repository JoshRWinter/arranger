#ifndef TARGA_H
#define TARGA_H

#include <stdexcept>

class Targa{
public:
	Targa(const char*);
	Targa(const Targa&) = delete;
	Targa(Targa&&) noexcept;
	virtual ~Targa();
	Targa &operator=(Targa&&) noexcept;
	virtual int get_width()const{return width;}
	virtual int get_height()const{return height;}
	virtual void get_bitmap(unsigned char*)const;

private:
	void bgr_to_rgb();

	int width;
	int height;
	unsigned char *data; // img data
};

#endif // TARGA_H
