"""Read published gym levels/stats/rewards from shared C logic for Web parity."""
import re

def sync_balance(root, gyms, roster):
    source=(root/'core/game.c').read_text()
    gyms['levelCap']=int(re.search(r'PP_MAX_LEVEL (\d+)',(root/'core/game.h').read_text())[1])
    keys=['pewter','cerulean','vermilion','celadon','fuchsia','saffron','cinnabar','viridian']
    stats=re.findall(r'\{"([^"]+)",(\d+),(\d+),(\d+),(\d+),(\d+)\}',source.split('const pp_gym_enemy pp_pewter_team',1)[1].split('const pp_gym_def pp_gyms',1)[0])
    rewards=re.findall(r'team,(\d+),(\d+),(\d+),(\d+),',source.split('const pp_gym_def pp_gyms',1)[1].split('const pp_gym_def *pp_gym_current',1)[0])
    assert len(stats)==24 and len(rewards)==8
    for i,key in enumerate(keys):
        gym=gyms[key]
        gym.update(zip(['firstClearCoins','rematchCoins','winXp','lossXp'],map(int,rewards[i])))
        gym['enemies']=[dict(name=name,level=int(lv),art=int(art),hp=int(hp),attack=int(atk),defense=int(df),nationalId=next(p['nationalId'] for p in roster if p['id']==int(art))) for name,lv,art,hp,atk,df in stats[i*3:i*3+3]]
    gyms.update({k:gyms['pewter'][k] for k in ['firstClearCoins','rematchCoins','winXp','lossXp']})
