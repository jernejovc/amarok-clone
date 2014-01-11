/****************************************************************************************
 * Copyright (c) 2003-2005 Max Howell <max.howell@methylblue.com>                       *
 * Copyright (c) 2005-2013 Mark Kretschmann <kretschmann@kde.org>                       *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Pulic License for more details.              *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#include "ASCIIAnalyzer.h"

#include "PaletteHandler.h"

#include <cmath>

#include <QPainter>
#include <QResizeEvent>


ASCIIAnalyzer* ASCIIAnalyzer::instance = 0;

ASCIIAnalyzer::ASCIIAnalyzer( QWidget *parent )
    : Analyzer::Base( parent )
    , m_columns( 0 )         //int
    , m_rows( 0 )            //int
{
    instance = this;
    setObjectName( "ASCII" );

    setMaximumWidth( MAX_COLUMNS * ( BLOCK_WIDTH + 1 ) - 1 );
    setFps( 30 );
}

void
ASCIIAnalyzer::initializeGL()
{
    // Disable depth test (all is drawn on a 2d plane)
    glDisable( GL_DEPTH_TEST );
}

void
ASCIIAnalyzer::resizeGL( int w, int h )
{
    glViewport( 0, 0, (GLint)w, (GLint)h );

    // Set up a 2D projection matrix
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0.0, (GLdouble)w, (GLdouble)h, 0.0, 0.0, 1.0 );

    const int oldRows = m_rows;

    // Rounded up so that the last column/line is covered if partially visible
    m_columns = std::min( std::ceil( (double)width() / ( BLOCK_WIDTH + 1 ) ), (double)MAX_COLUMNS );
    m_rows    = std::ceil( (double)height() / ( BLOCK_HEIGHT + 1 ) );

    m_scope.resize( m_columns );
    m_store.resize( m_columns );

    if( m_rows != oldRows )
    {
        m_barPixmap = QPixmap( BLOCK_WIDTH, m_rows * ( BLOCK_HEIGHT + 1 ) );

//         qDebug() << m_barPixmap.rect().x() << "," << m_barPixmap.rect().y() << "," << m_barPixmap.width() << "," << m_barPixmap.height() << "\n";
        //painter->drawText(m_barPixmap.rect(), Qt::AlignCenter, "[]");
        //painter->scale(m_barPixmap.width(), m_barPixmap.height());

        m_yscale.resize( m_rows + 1 );

        const float PRE = 1, PRO = 1; //PRE and PRO allow us to restrict the range somewhat

        for( int z = 0; z < m_rows; ++z )
            m_yscale[z] = 1 - ( log10( PRE + z ) / log10( PRE + m_rows + PRO ) );

        m_yscale[m_rows] = 0;

        determineStep();
        paletteChange( palette() );
    }

    drawBackground();
    analyze( m_scope );
}

void
ASCIIAnalyzer::determineStep()
{
    // falltime is dependent on rowcount due to our digital resolution (ie we have boxes/blocks of pixels)
    // I calculated the value 30 based on some trial and error

    const double fallTime = 50 * m_rows;
    m_step = double( m_rows * 80 ) / fallTime; //80 = ~milliseconds between signals with audio data
}

void
ASCIIAnalyzer::transform( QVector<float> &s ) //pure virtual
{
    for( int x = 0; x < s.size(); ++x )
        s[x] *= 2;

    float *front = static_cast<float*>( &s.front() );

    m_fht->spectrum( front );
    m_fht->scale( front, 1.0 / 20 );

    //the second half is pretty dull, so only show it if the user has a large analyzer
    //by setting to m_scope.size() if large we prevent interpolation of large analyzers, this is good!
    s.resize( m_scope.size() <= MAX_COLUMNS / 2 ? MAX_COLUMNS / 2 : m_scope.size() );
}

void
ASCIIAnalyzer::analyze( const QVector<float> &s )
{
    interpolate( s, m_scope );
}

void
ASCIIAnalyzer::paintGL()
{
    // y = 2 3 2 1 0 2
    //     . . . . # .
    //     . . . # # .
    //     # . # # # #
    //     # # # # # #
    //
    // visual aid for how this analyzer works.
    // y represents the number of blanks
    // y starts from the top and increases in units of blocks

    // m_yscale looks similar to: { 0.7, 0.5, 0.25, 0.15, 0.1, 0 }
    // if it contains 6 elements there are 5 rows in the analyzer

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    // Paint the background
    drawTexture( m_background.data(), 0, 0, 0, 0 );

    for( uint y, x = 0; x < (uint)m_scope.size(); ++x )
    {
        // determine y
        for( y = 0; m_scope[x] < m_yscale[y]; ++y )
            ;

        // this is opposite to what you'd think, higher than y
        // means the bar is lower than y (physically)
        if( ( float )y > m_store[x] )
            y = uint( m_store[x] += m_step );
        else
            m_store[x] = y;

        int xpos = x * ( BLOCK_WIDTH + 1 );
        //qDebug() << "Bar: " << QRect(x * ( BLOCK_WIDTH + 1 ), y * ( BLOCK_HEIGHT + 1 ), 0, y * ( BLOCK_HEIGHT + 1 ) );
        // REMEMBER: y is a number from 0 to m_rows, 0 means all blocks are glowing, m_rows means none are
        drawTexture( m_barTexture.data(), xpos, y * ( BLOCK_HEIGHT + 1 ), 0, y * ( BLOCK_HEIGHT + 1 ) );

        //qDebug() << "Top bar: " << QRect(x * ( BLOCK_WIDTH + 1 ), int( m_store[x] ) * ( BLOCK_HEIGHT + 1 ), 0, 0 );
        // Draw second top bar to "ease" transition
        int top_ypos = int( m_store[x] ) * ( BLOCK_HEIGHT + 1 );
        drawTexture( m_topSecondBarTexture.data(), xpos, top_ypos + BLOCK_HEIGHT + 1, 0, 0 );

        // Draw top bar
        drawTexture( m_topBarTexture.data(), xpos, top_ypos, 0, 0 );
    }

    // Paint the foreground
    // drawTexture( m_foreground.data(), 0, 0, 0, 0 );
}

void
ASCIIAnalyzer::drawTexture( Texture* texture, int x, int y, int sx, int sy )
{
    const GLfloat xf = x;
    const GLfloat yf = y;
    const GLfloat wf = texture->size.width() - sx;
    const GLfloat hf = texture->size.height() - sy;
    const GLfloat sxf = (GLfloat)sx / texture->size.width();
    const GLfloat syf = (GLfloat)sy / texture->size.height();

    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, texture->id );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    // Draw a textured quad
    glBegin(GL_QUADS);
    glTexCoord2f( sxf, syf ); glVertex2f( xf, yf );
    glTexCoord2f( sxf, 1.0 ); glVertex2f( xf, yf + hf );
    glTexCoord2f( 1.0, 1.0 ); glVertex2f( xf + wf, yf + hf );
    glTexCoord2f( 1.0, syf ); glVertex2f( xf + wf, yf );
    glEnd();

    glDisable( GL_TEXTURE_2D );
}

void
ASCIIAnalyzer::paletteChange( const QPalette& ) //virtual
{
    const QColor bg = palette().background().color();


//     if(palette().dark()) {
//         qDebug() << "Dark palette.";
//     }
//     const QColor bg = Qt::black;
    const QFont font ("Cantarell", 10);

    QPixmap topBar( BLOCK_WIDTH, BLOCK_HEIGHT );
    topBar.fill( bg );
    QPainter tbp ( &topBar );
    tbp.setPen(Qt::red);
    tbp.setBackground(palette().background().color());
    tbp.setFont(font);
    tbp.drawText(topBar.rect(), Qt::AlignCenter, ".");
    m_topBarTexture = QSharedPointer<Texture>( new Texture( topBar ) );
    //qDebug() << topBar.rect();

    QPixmap topSecondBar( BLOCK_WIDTH, BLOCK_HEIGHT );
    // red on top, black on bottom
    QLinearGradient gradient (BLOCK_WIDTH/2, 0, BLOCK_WIDTH/2, BLOCK_HEIGHT);
    gradient.setColorAt(0.5, Qt::red);
    gradient.setColorAt(1.0, Qt::darkGreen);
    qDebug() << gradient;
    topSecondBar.fill( bg );
    QPainter tsbp ( &topSecondBar );
    tsbp.setPen( QPen(gradient, BLOCK_WIDTH) );
    tsbp.setBrush( gradient );
    //tsbp.setBackground(bg);
    tsbp.setFont(font);
    tsbp.drawText(topSecondBar.rect(), Qt::AlignCenter, "o");
    m_topSecondBarTexture = QSharedPointer<Texture>( new Texture( topSecondBar ) );

    m_barPixmap.fill( bg );
    QPainter p( &m_barPixmap );
    p.setPen(Qt::darkGreen);
    p.setFont(font);
    //p.drawText(m_barPixmap.rect(), Qt::AlignCenter, "#");
    //m_barPixmap.fill( bg );

    for( int y = 0; y < m_rows; ++y ) {
        QRect rect (0, y * ( BLOCK_HEIGHT + 1 ), BLOCK_WIDTH, BLOCK_HEIGHT);
        //qDebug() << rect;
        p.drawText(rect, Qt::AlignCenter, "#");
        //p.fillRect( 0, y * ( BLOCK_HEIGHT + 1 ), BLOCK_WIDTH, BLOCK_HEIGHT, QColor( r + int( dr * y ), g + int( dg * y ), b + int( db * y ) ) );
    }

    m_barTexture = QSharedPointer<Texture>( new Texture( m_barPixmap ) );
    drawBackground();

    QPixmap foreground( width(), height() );
    // red on top, black on bottom
    QLinearGradient fg_grad(width()/2, 0, width()/2, height());
    fg_grad.setColorAt(0.0, QColor(0, 0, 0, 0.0));
    fg_grad.setColorAt(1.0, QColor(0, 0, 0, 0.25));

    QPainter fgp ( &foreground );
    fgp.setPen( QPen(fg_grad, width()) );
    fgp.setBrush( fg_grad );
    fgp.drawRect(rect());
    m_foreground = QSharedPointer<Texture>( new Texture( foreground ) );
}

void
ASCIIAnalyzer::drawBackground()
{
    const QColor bg = palette().background().color();
//     const QColor bg = Qt::black;
    QPixmap background( size() );
    background.fill( bg );

    m_background = QSharedPointer<Texture>( new Texture( background ) );
}
