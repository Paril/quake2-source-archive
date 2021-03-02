//*************************************************************************************************************
//*************************************************************************************************************
// File: kotshtml.cpp
//*************************************************************************************************************
//*************************************************************************************************************

#include "stdafx.h"
#include "kotsadmin.h"

#include "kotsadmindlg.h"
#include "doptions.h"

#include "../kuser/user.h"

//*************************************************************************************************************
//*************************************************************************************************************
// Function: WriteHeader
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::WriteHtmlHeader( CStdioFile &file, bool bKings ) 
{
	CUser   *user;
	CString str;
	CString temp;

	file.WriteString( "<!doctype html public \"-//w3c//dtd html 4.0 transitional//en\">\n" );
	file.WriteString( "<html>\n" );
	file.WriteString( "<head>\n" );
	file.WriteString( "   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n" );
	file.WriteString( "   <title>King of the Server Scoreboard</title>\n" );
	file.WriteString( "</head>\n" );
	file.WriteString( "<body text=\"#FFFFFF\" bgcolor=\"#000000\" link=\"#FFFFFF\" vlink=\"#FFFFFF\" alink=\"#FFFFFF\">\n" );
	file.WriteString( "<div align=\"center\">\n" );
	file.WriteString( "  <p><font size=\"2\">This is the King of the Server Scoreboard. It is updated by\n" );
	file.WriteString( "    the administrator of the KOTS Server.&nbsp; If you have questions or comments\n" );
	file.WriteString( "    about KOTS please email&nbsp; <a href=\"mailto:norb@planetquake.com\">Norb</a>\n" );
	file.WriteString( "    or <a href=\"mailto:mother@planetquake.com\">Mother</a>.&nbsp; Visit the KOTS \n" );
	file.WriteString( "    website at&nbsp; <a href=\"http://www.planetquake.com/nightbreed\" TARGET=\"_top\">http://www.planetquake.com/nightbreed</a>.\n" );
	file.WriteString( "    </font></p>\n" );
	file.WriteString( "\n" );
	file.WriteString( "</div>\n" );

	// Begin table
	file.WriteString( "<center>\n" );
	file.WriteString( "  <table 1 BGCOLOR=\"#6b798c\" BORDERCOLORLIGHT=\"#FFFFFF\" BORDERCOLORDARK=\"#000000\" align=\"center\" border=\"1\" >\n" );
	file.WriteString( "    <tr>\n" );

	if ( !bKings )
		file.WriteString( "      <td colspan=\"7\" align=\"center\"><font size=\"+2\" color=\"#FFFFFF\"><b>The Reigning King</b></font></td>\n" );
	else
		file.WriteString( "      <td colspan=\"2\" align=\"center\"><font size=\"+2\" color=\"#FFFFFF\"><b>The Reigning King</b></font></td>\n" );

	file.WriteString( "    </tr>\n" );
	file.WriteString( "    <tr bgcolor=\"#000080\"> \n" );

	// Write the King
	user = (CUser *)m_users[0];

	CleanName( user->m_name, temp );

	if ( !bKings )
		str.Format( "      <td colspan=\"7\" align=\"center\"><b><font color=\"#FFFFFF\" size=\"+4\">%s</font></b></td>\n", temp );
	else
		str.Format( "      <td colspan=\"2\" align=\"center\"><b><font color=\"#FFFFFF\" size=\"+4\">%s</font></b></td>\n", temp );

	file.WriteString( str );
	file.WriteString( "    </tr>\n" );

	// Write members and update
	CTime ctime = CTime::GetCurrentTime();

	if ( !bKings )
		str.Format( "      <td colspan=\"7\" align=\"center\"><font size=\"2\">%d members in the King's \n", m_users.GetSize() );
	else
		str.Format( "      <td colspan=\"2\" align=\"center\"><font size=\"2\">%d members in the King's \n", m_users.GetSize() );

	file.WriteString( "    <tr> \n" );
	file.WriteString( str );

	str = ctime.Format( "        court -&nbsp;last updated: %m/%d/%Y %H:%M </font></td>\n" );

	file.WriteString( str );
	file.WriteString( "    </tr>\n" );

	if ( bKings )
		return;

	// Column Headers
	file.WriteString( "    <tr> \n" );
	file.WriteString( "      <td> \n" );
	file.WriteString( "        <div align=\"center\"><b>rank</b> </div>\n" );
	file.WriteString( "      </td>\n" );
	file.WriteString( "      <td> \n" );
	file.WriteString( "        <div align=\"center\"><b>the player known as</b> </div>\n" );
	file.WriteString( "      </td>\n" );
	file.WriteString( "      <td> \n" );
	file.WriteString( "        <div align=\"center\"><b>level</b> </div>\n" );
	file.WriteString( "      </td>\n" );
	file.WriteString( "      <td> \n" );
	file.WriteString( "         <div align=\"center\"><b>total score/next level</b> </div>\n" );
	file.WriteString( "      </td>\n" );
	file.WriteString( "      <td> \n" );
	file.WriteString( "        <div align=\"center\"><b>frags</b> </div>\n" );
	file.WriteString( "      </td>\n" );
	file.WriteString( "      <td> \n" );
	file.WriteString( "        <div align=\"center\"><b>deaths</b> </div>\n" );
	file.WriteString( "      </td>\n" );
	file.WriteString( "      <td> \n" );
	file.WriteString( "        <div align=\"center\"><b>percent</b> </div>\n" );
	file.WriteString( "      </td>\n" );
	file.WriteString( "    </tr>\n" );
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: WriteHtmlPlayer
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::WriteHtmlPlayer( CStdioFile &file, int index ) 
{
	CUser   *user;
	CString str;
	CString temp;
	CString link;

	user = (CUser *)m_users[index];

	file.WriteString( "<tr> \n" );

	// Rank
	file.WriteString( "  <td> \n" );
	str.Format( "    <div align=\"right\">%d</div>\n", index + 1 );
	file.WriteString( str );
	file.WriteString( "  </td>\n" );

	// Name
	file.WriteString( "  <td> \n" );

	CleanName( user->m_name, temp );
	
	if ( PlayerHtml( index ) )
	{
		m_numstat++;

		str.Format( "    <div align=\"center\"><a href=\"kotsp%d.htm\">%s</a></div>\n", index, temp );
	}
	else		
		str.Format( "    <div align=\"center\">%s</div>\n", temp );

	file.WriteString( str );
	file.WriteString( "  </td>\n" );

	// Level
	file.WriteString( "  <td> \n" );
	str.Format( "    <div align=\"right\">%d</div>\n", user->Level() );
	file.WriteString( str );
	file.WriteString( "  </td>\n" );

	// Score
	file.WriteString( "  <td> \n" );
	str.Format( "    <div align=\"right\">%d/%d</div>\n", user->Score(), user->NextLevel() );
	file.WriteString( str );
 	file.WriteString( "  </td>\n" );

	// Kills
	file.WriteString( "  <td> \n" );
	str.Format( "    <div align=\"right\">%d</div>\n", user->m_kills );
	file.WriteString( str );
	file.WriteString( "  </td>\n" );

	// Deaths
	file.WriteString( "  <td> \n" );
	str.Format( "    <div align=\"right\">%d</div>\n", user->m_killed );
	file.WriteString( str );
	file.WriteString( "  </td>\n" );

	// Ratio
	file.WriteString( "  <td> \n" );
	str.Format( "    <div align=\"right\">%.1f%%</div>\n", user->Ratio() );
	file.WriteString( str );
	file.WriteString( "  </td>\n" );

	file.WriteString( "</tr>\n" );
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: WriteHtmlFooter
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::WriteHtmlFooter( CStdioFile &file, bool bMain, LPCTSTR last, LPCTSTR next ) 
{
	CString str;

	file.WriteString( "</table>\n" );
	file.WriteString( "</center>\n" );
	file.WriteString( "\n" );

	if ( lstrlen( last ) > 0 || lstrlen( next ) > 0 )
	{
		str = "<center>";

		if ( lstrlen( last ) > 0 )
		{
			str += "<a href=\"";
			str += last;
			str += "\">&lt;&lt; Back</a>   ";
		}								 
		str += "<a href=\"kotskings.htm\">Kots Kings</a>";

		if ( lstrlen( next ) > 0 )
		{
			str += "   <a href=\"";
			str += next;
			str += "\">Next >></a>";
		}
		str += "</center>\n";

		file.WriteString( str );
	}

	if ( bMain )
	{
		if ( m_pref->shtml.GetLength() > 2 )
			file.WriteString( m_pref->shtml );
	}
	file.WriteString( "</body>\n" );
	file.WriteString( "</html>\n" );
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnHtml
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::OnHtml() 
{
	int        x;
	int        count = 0;
	CUser      *user;
	CString    str;
	CString    curr;
	CString    last;
	CString    next;
	CString    temp;
	CString    path;
	CStdioFile file;

	if ( m_pref->soutdir.IsEmpty() || !m_pref->bhtml )
		return;

	if ( m_users.GetSize() <= 0 )
		return;

	path = m_pref->soutdir + RANKFILE;

	curr = RANKFILE;

	if ( !file.Open( path, CFile::modeCreate | CFile::modeWrite ) )
		return;

	WriteHtmlHeader( file );

	m_numstat = 0;
	m_numlvls = 0;

	for ( x = 0; x < m_users.GetSize(); x++ )
	{
		user = (CUser *)m_users[x];

		if ( m_pref->bsplit && count == m_pref->iperpage )
		{
			count = 0;

			if ( m_pref->inumpages > 0 )
			{
				if ( m_pref->inumpages > m_numlvls + 1 )
				 	next.Format( LEVELFILE, m_numlvls++ );
				else
				{
					next.Empty();
					break;
				}
			}
			else
			 	next.Format( LEVELFILE, m_numlvls++ );

			WriteHtmlFooter( file, m_numlvls == 1, last, next );
		
			file.Close();

			last = curr;
			curr = next;

			next.Empty();

			path = m_pref->soutdir + curr;

			if ( !file.Open( path, CFile::modeCreate | CFile::modeWrite ) )
				return;

			WriteHtmlHeader( file );
		}
		WriteHtmlPlayer( file, x );

		count++;
	}

	if ( m_pref->bsplit )
		WriteHtmlFooter( file, m_numlvls == 0, last, next );
	else
		WriteHtmlFooter( file, true );

	file.Close();

	OnKings();
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: WriteRow
//*************************************************************************************************************
//*************************************************************************************************************

void WriteRow( CStdioFile *pfile, LPCTSTR left, int x )
{
	CString str;

	str.Format( "<td>%s</td>\n", left );

	pfile->WriteString( str );

	pfile->WriteString( "<td>\n" );
	
	str.Format( "  <div align=\"right\">%d</div>\n", x );

	pfile->WriteString( str );
	
	pfile->WriteString( "</td>\n" );
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: PlayerHTML
//*************************************************************************************************************
//*************************************************************************************************************

bool CKotsAdminDlg::PlayerHtml( int index ) 
{
	long       x;
	CUser      *user;
	CString    str;
	CString    temp;
	CString    path;
	CStdioFile file;

	if ( m_pref->soutdir.IsEmpty() )
		return false;

	if ( index < 0 || index >= m_users.GetSize() )
		return false;

	user = (CUser *)m_users[index];

	if ( user->Level() < m_pref->ilevel )
		return false;

	str.Format( PLAYERFILE, index );

	path = m_pref->soutdir + str;

	if ( !file.Open( path, CFile::modeCreate | CFile::modeWrite ) )
		return false;

	file.WriteString( "<!doctype html public \"-//w3c//dtd html 4.0 transitional//en\">\n" );
	file.WriteString( "<html>\n" );
	file.WriteString( "<head>\n" );
	file.WriteString( "   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n" );
	file.WriteString( "   <title>King of the Server Player Closeup</title>\n" );
	file.WriteString( "</head>\n" );
	file.WriteString( "<body text=\"#FFFFFF\" bgcolor=\"#000000\" link=\"#FFFFFF\" vlink=\"#FFFFFF\" alink=\"#FFFFFF\">\n" );
	file.WriteString( "<center>\n" );
	file.WriteString( "  <p><font size=\"2\">This is a King of the Server Scoreboard Player Page. It is \n" );
	file.WriteString( "    updated by the administrator of the KOTS Server.&nbsp; If you have questions \n" );
	file.WriteString( "    or comments about KOTS please email&nbsp; <a href=\"mailto:norb@planetquake.com\">Norb</a> \n" );
	file.WriteString( "    or <a href=\"mailto:mother@planetquake.com\">Mother</a>.&nbsp; Visit the KOTS \n" );
	file.WriteString( "    website at&nbsp; <a href=\"http://www.planetquake.com/nightbreed\" TARGET=\"_top\">http://www.planetquake.com/nightbreed</a>. \n" );
	file.WriteString( "    </font></p>\n" );
	file.WriteString( "  </center>\n" );

	// Table
	file.WriteString( "<table COLS=2 WIDTH=\"50%\" bgcolor=\"#6B798C\" align=\"center\" border=\"1\" bordercolorlight=\"#FFFFFF\" bordercolordark=\"#000000\" >\n" );

	// Player Name
	file.WriteString( "  <tr bgcolor=\"#000080\"> \n" );
	file.WriteString( "    <td colspan=\"2\"> \n" );
	str.Format( "      <div align=\"center\"><b><font color=\"#FFFFFF\" size=\"+3\">%s</font></b></div>\n", user->m_name );
	file.WriteString( str );
	file.WriteString( "    </td>\n" );
	file.WriteString( "  </tr>\n" );

	// last update
	CTime ctime = CTime::GetCurrentTime();

	file.WriteString( "  <tr>\n" );
	file.WriteString( "    <td colspan=\"2\">\n" );
	str = ctime.Format( "      <div align=\"center\"> <font size=\"2\">last updated: %m/%d/%Y %H:%M </font></div>\n" );
	file.WriteString( str );
	file.WriteString( "    </td>\n" );
	file.WriteString( "  </tr>\n" );

	// Headings
	file.WriteString( "  <tr bgcolor=\"#000080\">\n" );
	file.WriteString( "    <td valign=\"top\" colspan=\"2\"><b>..player</b></td>\n" );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr>\n" );
	WriteRow( &file, "rank"   , index + 1     );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "level"  , user->Level( &x ) );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "score"        , user->Score()  );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "next level"  , x );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "points"  , user->m_points );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "hole"  , user->m_hole );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "health"     , user->m_health );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "armor"    , user->m_barmor );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "version"  , user->m_version );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	file.WriteString( "    <td>member since</td>\n" );
	file.WriteString( "    <td> \n" );
	CTime ftime( user->m_firstseen );
	str = ftime.Format( "      <div align=\"right\">%m/%d/%Y %H:%M</div>\n" );
	file.WriteString( str );
	file.WriteString( "    </td>\n" );

	file.WriteString( "  <tr bgcolor=\"#000080\">\n" );
	file.WriteString( "    <td valign=\"top\" colspan=\"2\"><b>..performance</b></td>\n" );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "frags"          , user->m_kills	   );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "fragged"      , user->m_killed	      );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "teleports"      , user->m_teles	      );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "suicide"      , user->m_suicide );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "longest frag spree", user->m_spree );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "frag sprees", user->m_spreecount );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "frag spree wars", user->m_spreewar );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "sprees broken", user->m_spreebroke );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "spree wars broken", user->m_spreewarbroke );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "2 fers", user->m_2fer );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	file.WriteString( "    <td>kill ratio</td>\n" );
	file.WriteString( "    <td> \n" );
	str.Format( "      <div align=\"right\">%.1f%%</div>\n", user->Ratio() );
	file.WriteString( str );
	file.WriteString( "    </td>\n" );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	file.WriteString( "    <td>last seen</td>\n" );
	file.WriteString( "    <td> \n" );
	CTime ttime( user->m_lastseen );
	str = ttime.Format( "      <div align=\"right\">%m/%d/%Y %H:%M</div>\n" );
	file.WriteString( str );
	file.WriteString( "    </td>\n" );

	file.WriteString( "    <tr> \n" );
	file.WriteString( "    <td>playing time</td>\n" );
	file.WriteString( "    <td> \n" );
	CTimeSpan stime( user->m_playtime );
	str = stime.Format( "      <div align=\"right\">%D days %H:%M:%S</div>\n" );
	file.WriteString( str );
	file.WriteString( "    </td>\n" );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr bgcolor=\"#000080\"> \n" );
	file.WriteString( "    <td colspan=\"2\"><b>..items</b> </td>\n" );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "teleportation balls", user->m_tballz );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "power cubes", user->m_power );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr bgcolor=\"#000080\"> \n" );
	file.WriteString( "    <td colspan=\"2\"><b>..ammo</b> </td>\n" );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "bullets", user->m_bullets	);
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "shells", user->m_shells );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "cells", user->m_cells );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "grenades"    , user->m_grenades );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "rockets"     , user->m_rockets );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "slugs"	   , user->m_slugs  );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr bgcolor=\"#000080\"> \n" );
	file.WriteString( "    <td valign=\"top\" colspan=\"2\"><b>..weapons</b></td>\n" );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "saber", user->m_blaster	);
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "shotgun", user->m_shotgun );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "super shotgun", user->m_sshotgun );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "machine gun", user->m_mgun	 );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "chain gun", user->m_cgun	 );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "grenade launcher", user->m_glauncher );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "rocket launcher", user->m_rlauncher	);
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "hyperblaster", user->m_hyperblaster );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "railgun", user->m_rgun		);
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr> \n" );
	WriteRow( &file, "bfg"	  , user->m_bfg	    );
	file.WriteString( "  </tr>\n" );

	file.WriteString( "  <tr bgcolor=\"#000080\"> \n" );
	file.WriteString( "    <td colspan=\"2\" > \n" );
	file.WriteString( "      <p><DIV align=center><b><a href=\"kotsdata.htm\">back to the rankings</a></b></DIV></p>\n" );
	file.WriteString( "      </td>\n" );
	file.WriteString( "  </tr>\n" );

	file.WriteString( " </table>\n" );
	file.WriteString( "</body>\n" );
	file.WriteString( "</html>\n" );

	file.Close();

	return true;
}

//*************************************************************************************
//*************************************************************************************
// Function: StringToHex
//*************************************************************************************
//*************************************************************************************

void CKotsAdminDlg::StringToHex( LPCTSTR source, CString &target )
{
	int  i, source_len;
	char hex_marker[2];
	char hex[4];

	target.Empty();

	strcpy( hex_marker, "%\0" );

	source_len = strlen( source );

	for ( i = 0; i < source_len; i++ )
	{
		strcpy( hex,"\0\0\0\0" );

		if ( ( source[i] < 65 || source[i] > 122 ) || ( source[i] > 90 && source[i] < 97 ) )
		{
			itoa( source[i], hex, 16 );

			target += hex_marker;
			target += hex;	
		}
		else
			target += source[i];
	}
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: WriteKingTitle
//*************************************************************************************************************
//*************************************************************************************************************

static void WriteKingTitle( CStdioFile &file, LPCTSTR title )
{
 	file.WriteString( "    <TR bgcolor=\"#000080\"> \n" );
	file.WriteString( "      <TD colspan=\"2\"><font face=\"Verdana, Arial, Helvetica, sans-serif\"><B>\n" );
	file.WriteString( title );
	file.WriteString( "      </B></font></TD>\n" );
	file.WriteString( "    </TR>\n" );
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: WriteKingInfo
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::WriteKingInfo( CStdioFile &file, LPCTSTR name, LPCTSTR data )
{
	CString str;
	CString link;
	CString temp;

	file.WriteString( "    <TR> \n" );
	file.WriteString( "      <TD height=\"40\"> \n" );
	file.WriteString( "        <DIV align=center><a \n" );

	CleanName  ( name, temp );
	StringToHex( name, link );
	
	str.Format( "      <font face=\"Verdana, Arial, Helvetica, sans-serif\">%s</font></a></DIV>\n", temp );
	file.WriteString( str );
	
	file.WriteString( "      </TD>\n" );
	file.WriteString( "      <TD height=\"40\"> \n" );

	str.Format( "        <DIV align=center><font face=\"Verdana, Arial, Helvetica, sans-serif\">%s</font></DIV>\n", data );
	file.WriteString( str );

	file.WriteString( "      </TD>\n" );
	file.WriteString( "    </TR>\n" );
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnKings
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::OnKings() 
{
	CUser      *user;
	CString    str;
	CString    path;
	CStdioFile file;

	if ( m_pref->soutdir.IsEmpty() || !m_pref->bhtml )
		return;

	if ( m_users.GetSize() <= 0 )
		return;

	path = m_pref->soutdir + KINGFILE;

	if ( !file.Open( path, CFile::modeCreate | CFile::modeWrite ) )
		return;

	WriteHtmlHeader( file, true );

	WriteKingTitle( file, "king of all servers - highest score\n" );

	user = (CUser *)m_users[0];

	str.Format( "%d", user->Score() );

	WriteKingInfo( file, user->m_name, str );

	if ( m_users.GetSize() >= 2 )
	{
		WriteKingTitle( file, "prince of all servers - 2nd highest score\n" );

		user = (CUser *)m_users[1];

		str.Format( "%d", user->Score() );

		WriteKingInfo( file, user->m_name, str );
	}

	if ( m_users.GetSize() >= 3 )
	{
		WriteKingTitle( file, "duke of all servers - 3rd highest score\n" );

		user = (CUser *)m_users[2];

		str.Format( "%d", user->Score() );

		WriteKingInfo( file, user->m_name, str );
	}
	
	if ( m_khole )
	{
		WriteKingTitle( file, "queen of all servers - highest hole\n" );

		user = m_khole;

		str.Format( "%d", user->m_hole );

		WriteKingInfo( file, user->m_name, str );
	}

	if ( m_kfrag )
	{
		WriteKingTitle( file, "knight of the rose - most frags\n" );

		user = m_kfrag;

		str.Format( "%d", user->m_kills );

		WriteKingInfo( file, user->m_name, str );
	}

	if ( m_kcheetah )
	{
		WriteKingTitle( file, "knight of the cheetah - most frags per minute\n" );

		user = m_kcheetah;

		str.Format( "%3.2f", user->KillsPerMin() );

		WriteKingInfo( file, user->m_name, str );
	}

	if ( m_kdeath )
	{
		WriteKingTitle( file, "jester of all servers - most fragged\n" );

		user = m_kdeath;

		str.Format( "%d", user->m_killed );

		WriteKingInfo( file, user->m_name, str );
	}

	if ( m_kspree )
	{
		WriteKingTitle( file, "knight of the lake - longest frag spree\n" );

		user = m_kspree;

		str.Format( "%d", user->m_spree );

		WriteKingInfo( file, user->m_name, str );
	}

	if ( m_kspreecount )
	{
		WriteKingTitle( file, "knight of the fire order - most frag sprees\n" );

		user = m_kspreecount;

		str.Format( "%d", user->m_spreecount );

		WriteKingInfo( file, user->m_name, str );
	}

	if ( m_kspreewar )
	{
		WriteKingTitle( file, "knight of the hellenic order - most frag spree wars\n" );

		user = m_kspreewar;

		str.Format( "%d", user->m_spreewar );

		WriteKingInfo( file, user->m_name, str );
	}

	if ( m_kspreebroke )
	{
		WriteKingTitle( file, "royal order of norb - most frag sprees broken\n" );

		user = m_kspreebroke;

		str.Format( "%d", user->m_spreebroke );

		WriteKingInfo( file, user->m_name, str );
	}

	if ( m_kspreewarbroke )
	{
		WriteKingTitle( file, "royal order of mother - most frag spree wars broken\n" );

		user = m_kspreewarbroke;

		str.Format( "%d", user->m_spreewarbroke );

		WriteKingInfo( file, user->m_name, str );
	}

	if ( m_k2fer )
	{
		WriteKingTitle( file, "knight of the dopplefragger - most 2-fers\n" );

		user = m_k2fer;

		str.Format( "%d", user->m_2fer );

		WriteKingInfo( file, user->m_name, str );
	}

	if ( m_kteles )
	{
		WriteKingTitle( file, "wizard of all servers - most teleports\n" );

		user = m_kteles;

		str.Format( "%d", user->m_teles );

		WriteKingInfo( file, user->m_name, str );
	}

	if ( m_ksuicide )
	{
		WriteKingTitle( file, "knight of the putrid odor - most suicides\n" );
	
		user = m_ksuicide;

		str.Format( "%d", user->m_suicide );

		WriteKingInfo( file, user->m_name, str );
	}

	if ( m_kratio )
	{
		WriteKingTitle( file, "knight of the fox - best kill ratio level 1 and above\n" );
	
		user = m_kratio;

		str.Format( "%.1f%%", user->Ratio() );

		WriteKingInfo( file, user->m_name, str );
	}

	if ( m_kfirstseen )
	{
		WriteKingTitle( file, "oldest member of the court - earliest member date\n" );

		user = m_kfirstseen;

		CTime ctime( user->m_firstseen );

		str = ctime.Format( "%m/%d/%Y %H:%M" );

		WriteKingInfo( file, user->m_name, str );
	}

	if ( m_kplaytime )
	{
		WriteKingTitle( file, "get a life - longest playing time\n" );

		user = m_kplaytime;

		CTimeSpan cspan( user->m_playtime );

		str = cspan.Format( "%D days %H:%M:%S" );

		WriteKingInfo( file, user->m_name, str );
	}

	file.WriteString( "    </TBODY> \n" );
	file.WriteString( "  </TABLE>\n" );

	file.WriteString( "<a href=\"kotsdata.htm\">Back To Rankings</a>\n" );

	file.WriteString( "</CENTER>\n" );
	file.WriteString( "</BODY></HTML>\n" );

	file.Close();
}


//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************


