// FHT - Fast Hartley Transform Class
//
// Copyright (C) 2004  Melchior FRANZ - mfranz@kde.org
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
//
// $Id$

#ifndef FHT_H
#define FHT_H

class FHT {
	int		m_exp2;
	int		m_num;
	float		*m_buf;
	float		*m_tab;
	int		*m_log;

	void		makeCasTable();
	void 		_transform(float *, int, int);

public:
	FHT(int);
	~FHT();
	inline int	sizeExp() const { return m_exp2; }
	inline int	size() const { return m_num; }
	float		*copy(float *, float *);
	float		*clear(float *);
	void		scale(float *, float);
	void		ewma(float *, float *, float);
	void		pattern(float *, bool);
	void		logSpectrum(float *, float *);
	void		semiLogSpectrum(float *);
	void		spectrum(float *);
	void		power(float *);
	void		power2(float *);
	void		transform8(float *);
	void		transform(float *);
};

#endif
