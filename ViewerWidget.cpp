#include   "ViewerWidget.h"

ViewerWidget::ViewerWidget(QString viewerName, QSize imgSize, QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);
	name = viewerName;
	if (imgSize != QSize(0, 0)) {
		img = new QImage(imgSize, QImage::Format_ARGB32);
		img->fill(Qt::white);
		resizeWidget(img->size());
		setPainter();
		setDataPtr();
	}
}
ViewerWidget::~ViewerWidget()
{
	delete painter;
	delete img;
}
void ViewerWidget::resizeWidget(QSize size)
{
	this->resize(size);
	this->setMinimumSize(size);
	this->setMaximumSize(size);
}

//Image functions
bool ViewerWidget::setImage(const QImage& inputImg)
{
	if (img != nullptr) {
		delete img;
	}
	img = new QImage(inputImg);
	if (!img) {
		return false;
	}
	resizeWidget(img->size());
	setPainter();
	update();

	return true;
}
bool ViewerWidget::isEmpty()
{
	if (img->size() == QSize(0, 0)) {
		return true;
	}
	return false;
}

//Data function
void ViewerWidget::setPixel(int x, int y, const QColor& color)
{
	if (isInside(x, y))
		data[x + y * img->width()] = color.rgb();
	else
		qDebug() << "nechce vykreslit bod" << x << "," << y << "\n";
}
void ViewerWidget::setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	if (isInside(x, y)) {
		QColor color(r, g, b);
		setPixel(x, y, color);
	}
}

void ViewerWidget::clear(QColor color)
{
	for (size_t x = 0; x < img->width(); x++)
	{
		for (size_t y = 0; y < img->height(); y++)
		{
			setPixel(x, y, color);
		}
	}
	update();
}



//Draw functions
void ViewerWidget::freeDraw(QPoint end, QPen pen)
{
	painter->setPen(pen);
	painter->drawLine(freeDrawBegin, end);

	update();
}

void ViewerWidget::clear()
{
	img->fill(Qt::white);
	update();
}



void ViewerWidget::usecka_DDA(QPointF A, QPointF B, QColor color) {
	int x, y, x2, y2;
	double chyba = 0;
	if (B.y() == A.y()) {
		int delta = abs(A.x() - B.x()), x1;
		if (A.x() < B.x())
			x1 = A.x();
		else
			x1 = B.x();
		for (x = 0; x < delta; x++)
			setPixel((int)x + (int)x1, (int)A.y(), color);
	}
	else {
		double smernica = (B.y() - A.y()) / (double)(B.x() - A.x());
		double posun = 0;
		if (abs(smernica) < 1) {											//riadiaca os x
			if (A.x() < B.x()) {
				x = A.x(); y = A.y(); x2 = B.x();
			}
			else if (A.x() > B.x()) {
				x = B.x(); y = B.y(); x2 = A.x();
			}
			else if (A.x() == B.x()) {
				int delta = abs(A.y() - B.y()), y1;
				if (A.y() < B.y())
					y1 = A.y();
				else
					y1 = B.y();
				for (y = 0; y < delta; y++)
					setPixel((int) (y + y1), (int)A.x(), color);
			}

			for (x; x < x2; x++) {
				setPixel((int)x, (int)y, color);
				posun += smernica;
				if (posun > 0.5) {
					y += (int)smernica;
					if ((int)smernica == 0)
						y++;
					posun += -1;
				}
				if (posun < -0.5) {
					y += -(int)smernica;
					if ((int)smernica == 0)
						y--;
					posun += 1;
				}
			}
		}
		else {																//riadiaca os y
			if (A.y() > B.y())
				y = A.y(), x = A.x(), y2 = B.y();
			else if (A.y() < B.y())
				y = B.y(), x = B.x(), y2 = A.y();
			for (y; y > y2; y--) {
				setPixel((int)x, (int)y, color);
				posun += 1 / smernica;
				if (posun > 0.5) {
					x--;
					posun += -1;
				}
				if (posun < -0.5) {
					x++;
					posun += 1;
				}
			}
		}
	}
	update();
}

void ViewerWidget::kresliHedron(QList<QPointF> naVykreslenie, QList<int> nevykresluj) {
	img->fill(Qt::white);
	update();
	int i, k;
	int w = img->width();
	int h = img->height();
	bool nekresli = false;
	QList<QPointF> W, V;
	for (i = 0; i < naVykreslenie.size(); i += 3) {
		nekresli = false;
		for (k = 0; k < nevykresluj.size(); k++) {
			if (i/3 == nevykresluj[k]) {
				nekresli = true;
			}
		}
		if (!nekresli) {
			V.clear();
			V.append(naVykreslenie[i]);
			V.append(naVykreslenie[i + 1]);
			V.append(naVykreslenie[i + 2]);
			QPointF S, P, A, B;
			int Xmin[4] = { 1, 1, -w + 1, -h + 1 };
			int xmin = 0, j = 0, i = 0;
			for (j = 0; j < 4; j++) {
				xmin = Xmin[j];
				if (V.size() > 0)
					S = V[V.size() - 1];
				for (i = 0; i < V.size(); i++) {
					if (V[i].x() >= xmin) {
						if (S.x() >= xmin) {
							W.push_back(V[i]);
						}
						else {
							P.setX(xmin);
							P.setY(S.y() + (xmin - S.x()) * (V[i].y() - S.y()) / (V[i].x() - S.x()));
							W.push_back(P);
							W.push_back(V[i]);
						}
					}
					else {
						if (S.x() >= xmin) {
							P.setX(xmin);
							P.setY(S.y() + (xmin - S.x()) * (V[i].y() - S.y()) / (V[i].x() - S.x()));
							W.push_back(P);
						}
					}
					S = V[i];
				}
				V.clear();
				for (i = 0; i < W.size(); i++) {
					P.setX(W[i].y());
					P.setY(-W[i].x());
					V.push_back(P);
				}
				W.clear();
			}
			for (i = 0; i < V.size(); i++) {
				A.setX(V[i].x());
				A.setY(V[i].y());
				B.setX(V[(i + 1) % (V.size())].x());
				B.setY(V[(i + 1) % (V.size())].y());
				if (!isnan(A.x()) && !isnan(B.x()) && !isnan(A.y()) && !isnan(B.y()))
					usecka_DDA(A, B, Qt::black);
			}
		}
	}
	update();
}

//Slots
void ViewerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	QRect area = event->rect();
	painter.drawImage(area, *img, area);
}

