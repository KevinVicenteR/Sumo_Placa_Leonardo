import sys, os, shutil, subprocess, json, hashlib, csv, time
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor, as_completed
SRC=Path('/Volumes/ssd-externo/02-Documentos/GitHub/Sumo_Placa_Leonardo/.claude/worktrees/x3-sin-arrancador')
TMP=Path('/tmp/codex_validacion_competencia')
OUT=Path('/Volumes/ssd-externo/02-Documentos/GitHub/Sumo_Placa_Leonardo/.claude/worktrees/simulaciones-30k-muestras-6a1c63/simulacion/resultados/codex_validacion_competencia')
OUT.mkdir(parents=True,exist_ok=True)
SNAP=TMP/'actual'
SNAP.mkdir(exist_ok=True)
for d in ('include','src','simulacion'):
    shutil.copytree(SRC/d,SNAP/d,dirs_exist_ok=True,ignore=shutil.ignore_patterns('build','resultados','__pycache__','.DS_Store'))
sys.path.insert(0,str(SNAP/'simulacion'))
import simular, bateria
manifest={str(p.relative_to(SNAP)):hashlib.sha256(p.read_bytes()).hexdigest() for d in ('include','src','simulacion') for p in (SNAP/d).rglob('*') if p.is_file() and '__pycache__' not in str(p)}
meta={'repo':str(SRC),'branch':subprocess.check_output(['git','branch','--show-current'],cwd=SRC,text=True).strip(),'head':subprocess.check_output(['git','rev-parse','HEAD'],cwd=SRC,text=True).strip(),'status':subprocess.check_output(['git','status','--porcelain'],cwd=SRC,text=True),'seeds':[424242,424313],'duration':30,'manifest_sha256':manifest}
(OUT/'metadatos.json').write_text(json.dumps(meta,indent=2))
refs=TMP/'referencia'
refs.mkdir(exist_ok=True)
shutil.copytree(SNAP/'include',refs/'include',dirs_exist_ok=True)
for f in ('estrategia/TablaRedLogica.h','movimiento/TablaRedControl.h'):
    (refs/'include'/f).write_bytes(subprocess.check_output(['git','show','pesos_neuronales_sin_arrancador:include/'+f],cwd=SRC))
casillas=[(r,rv,f) for f in bateria.FACTORES for r in bateria.ROUNDS for rv in bateria.RIVALES]
assert len(casillas)==420
for version,root in [('referencia',refs),('entrenada',SNAP)]:
    simular.RAIZ=root
    simular.BUILD=TMP/('build_'+version)
    simular.BUILD.mkdir(exist_ok=True)
    # Compile the same snapshot sources/support, swapping only copied include tables.
    if root==refs:
        (refs/'simulacion').mkdir(exist_ok=True)
        shutil.copytree(SNAP/'simulacion'/'support',refs/'simulacion'/'support',dirs_exist_ok=True)
    binary=simular.compilar({})
    print('compiled',version,flush=True)
    def run(cell):
        r,rv,f=cell
        rows=simular.ejecutar(binary,rv[1],72,bateria.ROBOT+['--dur','30','--semilla','424242']+r[1]+rv[2]+f[1])
        assert len(rows)==72 and [int(x['semilla']) for x in rows]==list(range(424242,424314))
        res=(sum(x['gano']=='1' for x in rows),sum(x['cayo']=='1' for x in rows),len(rows),sum(x['cayo']=='1' and x['empujado']!='1' for x in rows))
        return (r[0],rv[0],f[0]),res,rows
    results={}
    with (OUT/(version+'_combates.csv')).open('w') as fh:
        writer=None
        with ThreadPoolExecutor(max_workers=os.cpu_count()) as pool:
            futures=[pool.submit(run,c) for c in casillas]
            for i,future in enumerate(as_completed(futures),1):
                key,res,rows=future.result(); results[key]=res
                if writer is None:
                    writer=csv.DictWriter(fh,fieldnames=['round','rival','factor']+list(rows[0]));writer.writeheader()
                for row in rows: writer.writerow(dict(zip(['round','rival','factor'],key),**row))
                if i%42==0: print(version,i,'/420',flush=True)
    results={ (r[0],rv[0],f[0]):results[(r[0],rv[0],f[0])] for r,rv,f in casillas }
    bateria.guardar_json(results,OUT/(version+'.json'),{'semillas':[424242,424313],'combates_por_casilla':72,'duracion':30})
    def total(filter):
        v,c,n=bateria.sumar(results,filter)
        return {'combates':n,'victorias':v,'caidas':c,'propias':bateria.caidas_propias(results,filter),'gana_pct':100*v/n,'cae_pct':100*c/n}
    summary={'total':total(lambda *_:True),'ideal':total(lambda r,rv,f:f=='Ideal' and rv in ('quieto','deambula','embiste','flanquea')),'por_modo':{str(m):total(lambda r,rv,f,m=m:bateria.MODO_DE_ROUND[r]==m) for m in range(1,5)}}
    (OUT/(version+'_resumen.json')).write_text(json.dumps(summary,indent=2))
    print(version,json.dumps(summary),flush=True)
shutil.copy2(__file__,OUT/'validar.py')
