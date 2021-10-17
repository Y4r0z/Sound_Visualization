#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <cmath>
#include <vector>
#include <complex>
#include <valarray>

using namespace std;

void interpolate(vector<float> &ar, float power);
void hold(vector<float> prev, vector<float> &current, float power);
void fft(valarray<complex<double>> &x);

int main()
{
    string fileName = "Files/music5.ogg";
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Audio visualizer",sf::Style::Fullscreen,settings);

    int circleRadius = 250;
    int circleResolution = 256;
    int maxHeight = 250;

    valarray<complex<double>> complexArray;
    complexArray.resize(circleResolution);

    vector<float> leveler;
    for(int i = 0; i < circleResolution; i++)
    {
        leveler.push_back(0.1f +  abs(sin(2 * M_PI * i / circleResolution)));
    }

    sf::Vector2f centerPos = sf::Vector2f(window.getSize().x/2,window.getSize().y/2);

    vector<float> sizes;
    sizes.resize(circleResolution);

    vector<float> prevSizes;
    sizes.resize(circleResolution);

    sf::CircleShape mainCircle(circleRadius,256);
    mainCircle.setPosition(window.getSize().x/2 - circleRadius,window.getSize().y/2 - circleRadius);


    sf::ConvexShape shape;
    shape.setPointCount(circleResolution);
    shape.setPosition(centerPos);

    sf::ConvexShape shape2;
    shape2.setPointCount(circleResolution);
    shape2.setPosition(centerPos);




    sf::SoundBuffer buffer;
    buffer.loadFromFile(fileName);
    sf::Sound sound = sf::Sound(buffer);
    sound.play();

    int sampleCount = buffer.getSampleCount();
    int channelCount = buffer.getChannelCount();
    int sampleRate = buffer.getSampleRate() * channelCount;
    int sampleSumm = 0;


    while (window.isOpen())
    {
        sampleSumm = 0;
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return 0;
            }
            else if(sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::Right)
                {
                    sound.setPlayingOffset(sf::seconds(sound.getPlayingOffset().asSeconds() +  2.f));
                }
                if(event.key.code == sf::Keyboard::Left)
                {
                    sound.setPlayingOffset(sf::seconds(sound.getPlayingOffset().asSeconds() -  2.f));
                }
            }
        }

        prevSizes = sizes;

        int playProgress = sound.getPlayingOffset().asSeconds() * sampleRate;

        for(int i = playProgress; i < circleResolution + playProgress && i < sampleCount; i++)
        {
            complexArray[i-playProgress] = complex<double>(buffer.getSamples()[i],0.0);
        }
        fft(complexArray);

        for (int i = playProgress; i < circleResolution + playProgress && i < sampleCount; i++) {
            float height = (abs(complexArray[i - playProgress]) / 1000) *  leveler[i - playProgress];
            sampleSumm += height;
            if (height > maxHeight) height = maxHeight;

            sizes[i-playProgress] = 5 + height;

        }


        hold(prevSizes,sizes,0.9f);
        interpolate(sizes,0.8f);

        for(int i = 0; i < circleResolution; i++)
        {
            shape.setPoint(i,
                           ((float)circleRadius + sizes[i]) *
                           sf::Vector2f(sinf((i) * 2 * M_PI / circleResolution),
                                        cosf((i) * 2 * M_PI / circleResolution)
                           ));

            shape2.setPoint(i,
                           ((float)circleRadius + prevSizes[i] + maxHeight/12) *
                           sf::Vector2f(sinf((i) * 2 * M_PI / circleResolution),
                                        cosf((i) * 2 * M_PI / circleResolution)
                           ));
        }





        window.clear(sf::Color::Black);
        float k = (((float)sampleSumm/(float)circleResolution)/(float)maxHeight) * 3;
        shape2.setFillColor(sf::Color(255 - k * 100
                                      ,k * 50,
                                        k * 255));
        window.draw(shape2);
        window.draw(shape);
        mainCircle.setFillColor(sf::Color::Black);
        window.draw(mainCircle);
        window.display();
    }


    return 0;
}
float Lerp(float a, float b, float t)
{
    return a + (t*(b - a));

}
float Lerp(int a, int b, float t)
{
    return (float)a + (t*((float)b - (float)a));

}

void interpolate(vector<float> &ar, float power)
{
    for(int i = 1; i < ar.size(); i++)
    {
        float sizeSumm = (ar[i-1] + ar[i+1] + ar[i])/3;

        ar[i]= Lerp(ar[i], sizeSumm, power) ;
        ar[i]= Lerp(ar[i],ar[i-1], power/3);


    }
    ar[ar.size()-1] = ar[0];

    for(int i = ar.size()-2; i > 0; i--)
    {
        ar[i] = Lerp(ar[i], ar[i + 1], power/3);
    }


}

void hold(vector<float> prev, vector<float> &current,float power)
{
    if(prev.size() != current.size()) return;

    for(int i = 0; i < current.size(); i++)
    {
        if(prev[i] < current[i]) continue;

        current[i] = Lerp(current[i],prev[i],power);

    }
}

void fft(valarray<complex<double>> &x) {
    const int N = x.size();
    if (N <= 1) return;
    valarray<complex<double>> even = x[std::slice(0, N / 2, 2)];
    valarray<complex<double>> odd = x[std::slice(1, N / 2, 2)];
    fft(even);
    fft(odd);

    for (int k = 0; k < N / 2; k++) {
        complex<double> t = std::polar(1.0, -2 * M_PI * k / N) * odd[k];
        x[k] = even[k] + t;
        x[k + N / 2] = even[k] - t;
    }
}