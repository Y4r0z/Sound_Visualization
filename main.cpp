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
float getSum(vector<float> s, int a, int b);

int main()
{
    string fileName = "Files/bfg.ogg";
    sf::ContextSettings settings;
    settings.antialiasingLevel = 2;
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Audio visualizer",sf::Style::Fullscreen,settings);

    int circleRadius = 250;
    int circleResolution = 512;
    int chunkSamplesCount = 1024; //pow(2,n)
    int maxHeight = 250;
    int offset = 15;
    float holdPower = 0.9f;
    float interpolationPower = 0.9f;

    valarray<complex<double>> complexArray;
    complexArray.resize(chunkSamplesCount);

    vector<float> leveler;
    for(int i = 0; i < chunkSamplesCount; i++)
    {
        leveler.push_back(0.1f + ((float)circleResolution/(float)chunkSamplesCount) * abs(sin(2 * M_PI * i / chunkSamplesCount)));
    }

    sf::Vector2f centerPos = sf::Vector2f(window.getSize().x/2,window.getSize().y/2);

    vector<float> sizes;
    sizes.resize(chunkSamplesCount);

    vector<float> prevSizes;
    sizes.resize(chunkSamplesCount);

    sf::CircleShape mainCircle(circleRadius,256);
    mainCircle.setPosition(window.getSize().x/2 - circleRadius,window.getSize().y/2 - circleRadius);


    sf::VertexArray shape3(sf::TriangleFan, circleResolution+2);
    shape3[0].position = centerPos;

    sf::VertexArray shape4(sf::TriangleFan, circleResolution+2);
    shape4[0].position = centerPos;


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

        for(int i = playProgress; i < chunkSamplesCount + playProgress && i < sampleCount; i++)
        {
            complexArray[i-playProgress] = complex<double>(buffer.getSamples()[i],0.0);
        }
        fft(complexArray);

        for (int i = 0; i < chunkSamplesCount && i < sampleCount; i++) {
            float height = (abs(complexArray[i]) / 1000) *  leveler[i];
            sampleSumm += height;
            if (height > maxHeight) height = maxHeight;

            sizes[i] = height;

        }


        hold(prevSizes,sizes, holdPower);
        interpolate(sizes, interpolationPower);

        for(int i = 0; i < circleResolution; i++)
        {
            sf::Vector2f posOffset = ((float)circleRadius + offset + getSum(sizes,
                                                                i * (chunkSamplesCount / circleResolution),
                                                                i * (chunkSamplesCount / circleResolution) + (chunkSamplesCount / circleResolution))) *
                               sf::Vector2f(sinf((i) * 2 * M_PI / circleResolution),
                                            cosf((i) * 2 * M_PI / circleResolution)
                               );
            sf::Vector2f posOffset2 = ((float)circleRadius + offset + getSum(prevSizes,
                                                                            i * (chunkSamplesCount / circleResolution),
                                                                            i * (chunkSamplesCount / circleResolution) + (chunkSamplesCount / circleResolution))) *
                                     sf::Vector2f(sinf((i) * 2 * M_PI / circleResolution),
                                                  cosf((i) * 2 * M_PI / circleResolution)
                                     );


            shape3[i+1].position = centerPos + posOffset;
            shape4[i+1].position = centerPos + posOffset2 * 1.04f;
            /*
            shape.setPoint(i,
                           ((float)circleRadius + getSum(sizes,
                                                         i * (chunkSamplesCount / circleResolution),
                                                         i * (chunkSamplesCount / circleResolution) + (chunkSamplesCount / circleResolution))) *
                           sf::Vector2f(sinf((i) * 2 * M_PI / circleResolution),
                                        cosf((i) * 2 * M_PI / circleResolution)
                           ));

            shape2.setPoint(i,
                           ((float)circleRadius + getSum(prevSizes,
                                                         i * (chunkSamplesCount / circleResolution),
                                                         i * (chunkSamplesCount / circleResolution) + (chunkSamplesCount / circleResolution)) + maxHeight/14) *
                           sf::Vector2f(sinf((i) * 2 * M_PI / circleResolution),
                                        cosf((i) * 2 * M_PI / circleResolution)
                           ));
            */

        }
        shape3[circleResolution+1].position = shape3[1].position;
        shape4[circleResolution+1].position = shape4[4].position;
        float k = (((float)sampleSumm/(float)chunkSamplesCount)/(float)maxHeight) * 3;
        for (int i = 0; i < circleResolution+2; ++i)
        {
            shape4[i].color = sf::Color(255 - k * 100,k * 50,k * 255);
        }

        window.clear(sf::Color::Black);
        /*shape2.setFillColor(sf::Color(255 - k * 100
                ,k * 50,
                                        k * 255));
        window.draw(shape2);
        window.draw(shape);
        */
        window.draw(shape4);
        window.draw(shape3);
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
        //float sizeSumm = (ar[i-1] + ar[i+1] + ar[i])/3;
        //ar[i]= Lerp(ar[i], sizeSumm, power) ;
        ar[i]= Lerp(ar[i],ar[i-1], power);
    }
    ar[ar.size()-1] = ar[0];
    for(int i = ar.size()-2; i > 0; i--)
    {
        ar[i] = Lerp(ar[i], ar[i + 1], power);
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
        complex<double> t = std::polar(1.0, 2 * M_PI * k / N) * odd[k];
        x[k] = even[k] + t;
        x[k + N / 2] = even[k] - t;
    }
}

float getSum(vector<float> s,int a, int b)
{
    float sum = 0;
    for(int i = a; i < b; i++)
    {
        sum+=s[i];
    }
    return sum/(b-a);
}