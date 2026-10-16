import sys, os, shutil, subprocess, hashlib, json, csv, time
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
ROOT=Path.cwd(); TEMP=Path(__file__).resolve().parent
sys.dont_write_bytecode=True
sys.path.insert(0,str(ROOT/'simulacion'))
import simular, bateria
bateria.ROUNDS=bateria.ROUNDS_RUMANIA
OUT=ROOT/'simulacion/resultados'
N=52
cells=[(r,rv,f) for f in bateria.FACTORES for r in bateria.ROUNDS for rv in bateria.RIVALES]
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def inventory():
    paths=sorted(list((ROOT/'include').rglob('*'))+list((ROOT/'src').rglob('*'))+list((ROOT/'simulacion').glob('*.py'))+list((ROOT/'simulacion').glob('*.cpp'))+list((ROOT/'simulacion').glob('*.h'))+list((ROOT/'simulacion/support').rglob('*')))
    return {str(p.relative_to(ROOT)):sha(p) for p in paths if p.is_file()}
initial=inventory()
meta={'head':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),'n':N,'casillas':len(cells),'total_por_version':N*len(cells),'duracion':30,'seed_inicio':1000001,'seed_fin':1000000+N*len(cells),'script':str(Path(__file__)),'fuentes':initial,'compilacion':{}}
subprocess.run(['git','status','--porcelain=v1','-uall'],stdout=(TEMP/'status_before.txt').open('w'),check=True)
all_seeds={}
for version in ['entrenada']:
    root=TEMP/version; root.mkdir(); shutil.copytree(ROOT/'include',root/'include')
    (root/'simulacion').mkdir(); (root/'simulacion/support').symlink_to(ROOT/'simulacion/support',target_is_directory=True)
    if version=='referencia':
        for rel in ['include/estrategia/TablaRedLogica.h','include/movimiento/TablaRedControl.h']:
            (root/rel).write_bytes(subprocess.check_output(['git','show','HEAD:'+rel]))
    actual={str(p.relative_to(root)):sha(p) for p in (root/'include').rglob('*') if p.is_file()}
    different=[p for p,h in actual.items() if initial[p]!=h]
    assert set(different)==({'include/estrategia/TablaRedLogica.h','include/movimiento/TablaRedControl.h'} if version=='referencia' else set()),different
    simular.RAIZ=root; simular.BUILD=root/'build'; simular.BUILD.mkdir()
    binary=simular.compilar({})
    meta['compilacion'][version]={'binario':str(binary),'sha256':sha(binary),'include_sha256':actual,'diferencias_con_arbol_actual':different,'comando':['c++','-std=c++17','-O2','-w','-I'+str(next(simular.BUILD.glob('include_*'))),'-I'+str(root/'simulacion/support'),*map(str,simular.FUENTES),'-o',str(binary)]}
    print('Compilada',version,flush=True)
    def run(item):
        i,(r,rv,f)=item; seed=1000001+i*N
        extra=bateria.ROBOT+['--dur','30']+r[1]+rv[2]+f[1]+['--semilla',str(seed)]
        rows=simular.ejecutar(binary,rv[1],N,extra)
        seeds=[int(x['semilla']) for x in rows]
        assert seeds==list(range(seed,seed+N)),(i,seeds)
        res=(sum(x['gano']=='1' for x in rows),sum(x['cayo']=='1' for x in rows),len(rows),sum(x['cayo']=='1' and x['empujado']!='1' for x in rows))
        return (r[0],rv[0],f[0]),res,seeds,rows
    results={}; seeds_all=[]
    with (OUT/f'codex_rumania_{version}_combates.csv').open('w') as fh:
        writer=None
        with ThreadPoolExecutor(max_workers=min(8,os.cpu_count() or 4)) as pool:
            for i,(key,res,seeds,rows) in enumerate(pool.map(run,enumerate(cells))):
                results[key]=res; seeds_all.extend(seeds)
                for row in rows:
                    full=dict(round=key[0],rival=key[1],factor=key[2],**row)
                    if writer is None: writer=csv.DictWriter(fh,fieldnames=list(full)); writer.writeheader()
                    writer.writerow(full)
                if (i+1)%49==0: print(version,i+1,'/',len(cells),flush=True)
    assert len(seeds_all)==30576 and len(set(seeds_all))==30576 and min(seeds_all)>52
    all_seeds[version]=seeds_all
    bateria.guardar_json(results,OUT/f'codex_rumania_{version}.json',{'semillas_nuevas':True,'semilla_inicio':min(seeds_all),'semilla_fin':max(seeds_all),'combates_por_casilla':N})
assert len(all_seeds['entrenada'])==30576
assert inventory()==initial,'Fuentes cambiaron durante la validacion'
meta['verificacion']={'semillas_identicas':None,'semillas_fuera_de_1_a_52':True,'fuentes_sin_cambios':True,'semillas_sha256':hashlib.sha256(json.dumps(all_seeds['entrenada']).encode()).hexdigest()}
(OUT/'codex_rumania_validacion_entrenada.json').write_text(json.dumps(meta,ensure_ascii=False,indent=2))
shutil.copyfile(__file__,OUT/'codex_rumania_script_entrenada.py')
print('VALIDACION COMPLETA',flush=True)
